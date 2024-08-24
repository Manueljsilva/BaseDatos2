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


void operator >>(ifstream&stream , Alumno &p){
    stream.read(p.codigo, 5); 
    stream.read(p.nombre, 11);
    stream.read(p.apellidos, 20);
    stream.read(p.carrera, 15);
    stream.read((char*)&p.ciclo, sizeof(int));
    stream.read((char*)&p.mensualidad, sizeof(float));
    stream.read((char*)&p.next, sizeof(int));
}
void operator <<(ofstream&stream , Alumno &p){
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
        ifstream file(filename, ios::in | ios::binary);
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
        ofstream file(filename, ios::out | ios::in | ios::binary);
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
        ifstream file(filename, ios::binary);
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
        ofstream file(filename, ios::in | ios::out | ios::binary);
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
    int header = -1; // indica la posicion del primer registro libre

public:
    FixedRecordWithFreeList(string filename): FixedRecord(filename) {}
    vector<Alumno> load() override{
        vector<Alumno> alumnos;
        ifstream file(filename, ios::in | ios::binary);
        //implementar

        return alumnos;
    }
    void add(Alumno record) override{
        ofstream file(filename, ios::in | ios::out | ios::binary);
        if(!file.is_open()){
            cout<<"No se puede abrir el archivo" <<endl ; 
            exit(1);
        }
        if(header == -1){
            file.seekp(0, ios::end);
            file << record;
        }else{
            // leer el registro libre
        }
        file.close();
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


int main(){
    cout << "--------------------------------probando movethelast--------------------------------" << endl;
    PruebaMoveTheLast();

    return 0 ; 
}