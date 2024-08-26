#include<iostream>
#include<fstream>
#include<vector>
#include<string>

using namespace std; 

#pragma pack(push, 1) // los miembros de la estructura se colocarán en la memoria uno tras otro sin introducir bytes de relleno (ALINEAMIENTO DE 1 byte)
// asegura que la estructura "Alumno" sea empaquetada sin ningún byte de relleno, esto permite que el tamaño de la estructura sea exactamente la suma de los tamaños de sus miembros.
// De esta manera el tamaño y los datos en la estructura coincide con lo que se escribe y lee en el archivo binario.
struct Alumno{
    char codigo[5]; 
    char nombre[11];
    char apellidos[20];
    char carrera[15];
    int ciclo;
    float mensualidad;
    int next = -2;
};
#pragma pack(pop) // Restaurar el estado anterior del empaquetado


void operator >>(fstream&stream , Alumno &p){
    stream.read(p.codigo, 5); 
    stream.read(p.nombre, 11);
    stream.read(p.apellidos, 20);
    stream.read(p.carrera, 15);
    stream.read((char*)&p.ciclo, sizeof(int));
    stream.read((char*)&p.mensualidad, sizeof(float));
    stream.read((char*)&p.next, sizeof(int));
}
void operator <<(fstream&stream , Alumno &p){
    stream.write(p.codigo, 5);
    stream.write(p.nombre, 11);
    stream.write(p.apellidos, 20);
    stream.write(p.carrera, 15);
    stream.write((char*)&p.ciclo, sizeof(int));
    stream.write((char*)&p.mensualidad, sizeof(float));
    stream.write((char*)&p.next, sizeof(int));
}

//clase padre
class FixedRecord{
protected:
    string filename ; 
public:
    FixedRecord(string filename): filename(filename) {
        ofstream file(filename, ios::app | ios::binary);
        if(!file.is_open()){
            cout<<"No se puede abrir el archivo" <<endl ; 
            exit(1);
        }
        file.close();
    }
    virtual vector<Alumno> load() = 0;
    virtual void add(Alumno alumno) = 0;
    virtual Alumno readRecord(int pos) = 0;
    virtual bool deleteRecord(int pos) = 0;
    virtual int CantReg() = 0;
};

//clase hija
class FixedRecordWithMoveTheLast: public FixedRecord{
private:
    int candetele = 0;
public:
    FixedRecordWithMoveTheLast(string filename): FixedRecord(filename) {}

    vector<Alumno> load() override {
        vector<Alumno> alumnos;
        fstream file(filename, ios::in | ios::binary);
        if (!file.is_open()) throw ("No se pudo abrir el archivo");
        if (CantReg() == 0) {
            cout << "No hay registros" << endl;
            return alumnos;
        }
        file.seekg(0, ios::beg);

        int totalRecords = CantReg();
        for (int i = 0; i < totalRecords; i++) {
            Alumno al;
            file >> al;
            alumnos.push_back(al);
        }
        file.close();
        return alumnos;
    }

    void add(Alumno record) override{
        fstream file(filename, ios::out | ios::in | ios::binary);
        if(!file.is_open()){
            cout<<"No se puede abrir el archivo" <<endl ; 
            exit(1);
        }

        // si hay un registro eliminado, se reutiliza su posición
        if(candetele > 0){
            int posToReuse = CantReg(); // aquí capturo la posición del último registro
            file.seekp(posToReuse * sizeof(Alumno), ios::beg); // redirecciono el cursor en la posición a reutilizar
            candetele--; // reduzco contador de registros eliminados
        }
        // si no hay registros eliminados, agrego al final
        else {
            file.seekp(0, ios::end);
        }

        file << record;
        file.close();
    }

    Alumno readRecord(int pos) override{
        Alumno record;
        fstream file(filename, ios::binary | ios::in);
        if(!file.is_open()){
            cout<<"No se puede abrir el archivo_r" <<endl ; 
            exit(1);
        };

        file.seekg(pos * (sizeof(Alumno)), ios::beg);
        file >> record;
        file.close();
        return record;
    }

    bool deleteRecord(int pos) override{
        fstream file(filename, ios::in | ios::out | ios::binary);
        if(!file.is_open()) throw ("No se pudo abrir el archivo");
        
        int AllReg = CantReg();
        if(pos >= AllReg) {
            cout << "No se puede eliminar el registro" << endl;
            file.close();
            return false;
        }
        int lastPos = AllReg - 1;
        Alumno lastRecord = readRecord(lastPos);

        if (pos != lastPos){
            file.seekp(pos * sizeof(Alumno), ios::beg);
            file << lastRecord;
        }

        candetele++;
        file.close();
        return true;
    }

    int CantReg() override {
        ifstream file(filename, ios::binary | ios::in);
        file.seekg(0, ios::end);
        int size = file.tellg();
        file.close();
        return (size / sizeof(Alumno)) - candetele;
    }

    //destructor
    ~FixedRecordWithMoveTheLast(){
        //remove(filename.c_str());
    }
};

// Alexis

class FixedRecordWithFreeList: public FixedRecord{
private:
    int header; // indica que no hay registros eliminados

public:
    FixedRecordWithFreeList(string filename): FixedRecord(filename), header(-1) {}
    vector<Alumno> load() override{
        vector<Alumno> alumnos;
        ifstream file(filename, ios::in | ios::binary);
        if (!file.is_open()){
            cout<<"No se puede abrir el archivo" <<endl ; 
            exit(1);
        }
        file.seekg(0, ios::beg);
        while(true){
            Alumno al ; 
            streampos pos = file.tellg();
            file.read((char*)&al, sizeof(Alumno));
            // si se llega al final del archivo, salir del bucle
            if(file.eof()) break;
            //verificar si el registro está eliminado
            if(al.next == -2){
                alumnos.push_back(al);
            }
        }

        return alumnos;
    }
    //ofstream es para escribir en el archivo
    void add(Alumno record) override{
        fstream file(filename, ios::in | ios::out | ios::binary);
        if(!file.is_open()){
            cout<<"No se puede abrir el archivo" <<endl ; 
            exit(1);
        }
        int head = header;
        if (head == -1) {  // No hay registros eliminados, agregar al final
            file.seekp(0, ios::end);
            file << record;
        } else {  
            //buscar la posicion de next en el registro eliminado
            file.seekg(head * sizeof(Alumno) + sizeof(Alumno) - sizeof(int), ios::beg);
            // Leer la posición del siguiente registro eliminado
            int next;
            file.read((char*)&next, sizeof(int));
            // Actualizar el head de la free list
            header = next;
            // Escribir el registro en la posición del head
            file.seekp(head * sizeof(Alumno), ios::beg);
            file << record;
        }
        file.close();
    }

    Alumno readRecord(int pos) override{
        Alumno record;
        fstream file(filename, ios::binary | ios::in);
        if(!file.is_open()){
            cout<<"No se puede abrir el archivo_r" <<endl ; 
            exit(1);
        };
        //solo cargar el registro si no está eliminado osea los que possen un next = -2
        file.seekg(pos * sizeof(Alumno) + sizeof(Alumno) - sizeof(int), ios::beg);
        int next;
        file.read((char*)&next, sizeof(int));
        if (next != -2) {
            cout << "Registro eliminado" << endl;
            exit(1);
        }
        file.seekg(pos * sizeof(Alumno), ios::beg);
        file >> record;
        file.close(); 
        return record;
    }

    bool deleteRecord(int pos) override{
        ofstream file(filename, ios::in | ios::out | ios::binary);
        if(!file.is_open()){
            cout<<"No se puede abrir el archivo" <<endl ; 
            exit(1);
        }
        file.seekp(pos * sizeof(Alumno) + sizeof(Alumno) - sizeof(int), ios::beg);
        int next = header;
        file.write((char*)&next, sizeof(int));
        header = pos;
        file.close();
        return true;
    }

    int CantReg() override {
        // falta
        return 0;
    }

};

// prueba de la clase MoveTheLast
void PruebaMoveTheLast(){
    FixedRecord* file = new FixedRecordWithMoveTheLast("alumnos.dat");
    //FixedRecordWithMoveTheLast file("alumnos.dat");
    
    Alumno a1 = {"1234", "Juan", "Perez", "Sistemas", 5, 200.0};
    Alumno a2 = {"5678", "Maria", "Lopez", "Industrial", 6, 250.0};
    Alumno a3 = {"9101", "Pedro", "Gomez", "Civil", 7, 300.0};
    Alumno a4 = {"1121", "Ana", "Torres", "Mecanica", 8, 350.0};

    //agregando alumnos
    file->add(a1);
    file->add(a2);
    file->add(a3);
    file->add(a4);

    //cout << "Cantidad de registros: " << file->CantReg() << endl;

    //leyendo alumnos
    cout<< "-------Leyendo los alumnos-------" << endl;
    vector<Alumno> alumnos = file->load();
    for(Alumno a: alumnos){
        cout << a.codigo << " " << a.nombre << " " << a.apellidos << " " << a.carrera << " " << a.ciclo << " " << a.mensualidad << endl;
    }

    cout<<"---------------------------------" << endl;
    //leyendo un registro especifico 
    int pos = 2;
    cout << "Leyendo el registro en la posicion " << pos << ":" << endl; 
    Alumno a = file->readRecord(pos);
    cout << a.codigo << " " << a.nombre << " " << a.apellidos << " " << a.carrera << " " << a.ciclo << " " << a.mensualidad << endl;

    cout<<"---------------------------------" << endl;
    //eliminando un registro
    int posElim = 0;
    cout << "Eliminando el registro " << posElim << endl;
    file->deleteRecord(posElim);

    cout << "Cantidad de registros: " << file->CantReg() << endl;

    cout<<"---------------------------------" << endl;
    //leyendo alumnos
    cout<< "-------Leyendo los alumnos despues de eliminar un registro-------" << endl;
    alumnos = file->load();
    for(Alumno a: alumnos){
        cout << a.codigo << " " << a.nombre << " " << a.apellidos << " " << a.carrera << " " << a.ciclo << " " << a.mensualidad << endl;
    }

    cout << "---------------------------------" << endl;
    //agregando un nuevo registro
    cout << "Agregando un nuevo registro" << endl;
    Alumno a5 = {"1313", "Luis", "Garcia", "Mecatronica", 9, 400.0};
    file->add(a5);

    cout << "Cantidad de registros: " << file->CantReg() << endl;

    cout << "---------------------------------" << endl;
    //leyendo alumnos
    cout<< "-------Leyendo los alumnos despues de agregar un registro-------" << endl;
    alumnos = file->load();
    for(Alumno a: alumnos){
        cout << a.codigo << " " << a.nombre << " " << a.apellidos << " " << a.carrera << " " << a.ciclo << " " << a.mensualidad << endl;
    }

    cout << "Cantidad de registros: " << file->CantReg() << endl;
}

// prueba de la clase FreeList
void PruebaFreeList(){
    FixedRecord* file = new FixedRecordWithFreeList("alumnos.dat");
    //FixedRecordWithFreeList file("alumnos.dat");
    
    Alumno a1 = {"1234", "Juan", "Perez", "Sistemas", 5, 200.0};
    Alumno a2 = {"5678", "Maria", "Lopez", "Industrial", 6, 250.0};
    Alumno a3 = {"9101", "Pedro", "Gomez", "Civil", 7, 300.0};
    Alumno a4 = {"1121", "Ana", "Torres", "Mecanica", 8, 350.0};

    //agregando alumnos
    file->add(a1);
    file->add(a2);
    file->add(a3);
    file->add(a4);

    cout << "Cantidad de registros: " << file->CantReg() << endl;

    //leyendo alumnos
    cout<< "-------Leyendo los alumnos-------" << endl;
    vector<Alumno> alumnos = file->load();
    for(Alumno a: alumnos){
        cout << a.codigo << " " << a.nombre << " " << a.apellidos << " " << a.carrera << " " << a.ciclo << " " << a.mensualidad << endl;
    }

    cout<<"---------------------------------" << endl;
    //leyendo un registro especifico 
    int pos = 2;
    cout << "Leyendo el registro en la posicion " << pos << ":" << endl; 
    Alumno a = file->readRecord(pos);
    cout << a.codigo << " " << a.nombre << " " << a.apellidos << " " << a.carrera << " " << a.ciclo << " " << a.mensualidad << endl;

    cout<<"---------------------------------" << endl;
    //eliminando un registro
    int posElim = 0;
    cout << "Eliminando el registro " << posElim << endl;
    file->deleteRecord(posElim);

    //cout << "Cantidad de registros: " << file->CantReg() << endl;

    cout<<"---------------------------------" << endl;
    //leyendo alumnos
    cout<< "-------Leyendo los alumnos despues de eliminar un registro-------" << endl;
    alumnos = file->load();
    for(Alumno a: alumnos){
        cout << a.codigo << " " << a.nombre << " " << a.apellidos << " " << a.carrera << " " << a.ciclo << " " << a.mensualidad << endl;
    }

    cout << "---------------------------------" << endl;
    //agregando un nuevo registro
    cout << "Agregando un nuevo registro" << endl;
    Alumno a5 = {"1313", "Luis", "Garcia", "Mecatronica", 9, 400.0};
    file->add(a5);

    //cout << "Cantidad de registros: " << file->CantReg() << endl;

    cout << "---------------------------------" << endl;
    //leyendo alumnos
    cout<< "-------Leyendo los alumnos despues de agregar un registro-------" << endl;
    alumnos = file->load();
    for(Alumno a: alumnos){
        cout << a.codigo << " " << a.nombre << " " << a.apellidos << " " << a.carrera << " " << a.ciclo << " " << a.mensualidad << endl;
    }

    //cout << "Cantidad de registros: " << file->CantReg() << endl;
}

int main(){
    cout << "--------------------------------probando movethelast--------------------------------" << endl;
    PruebaMoveTheLast();
    cout << "--------------------------------probando freelist--------------------------------" << endl;
    PruebaFreeList();

    return 0 ; 
}