#include<iostream>
#include<fstream>
#include<vector>
#include<string>


using namespace std; 



struct Alumno{
    char codigo[5]; 
    char nombre[11];
    char apellidos[20];
    char carrera[15];
    int ciclo;
    float mensualidad;
};

void operator >>(ifstream&stream , Alumno &p){
    stream.read(p.codigo, 5); 
    stream.read(p.nombre, 11);
    stream.read(p.apellidos, 20);
    stream.read(p.carrera, 15);
    stream.read((char*)&p.ciclo, sizeof(int));
    stream.read((char*)&p.mensualidad, sizeof(float));
}
void operator <<(ofstream&stream , Alumno &p){
    stream.write(p.codigo, 5);
    stream.write(p.nombre, 11);
    stream.write(p.apellidos, 20);
    stream.write(p.carrera, 15);
    stream.write((char*)&p.ciclo, sizeof(int));
    stream.write((char*)&p.mensualidad, sizeof(float));
}

enum DeleteType{
    MOVE_THE_LAST,
    FREE_LIST
};

class FixedRecord{
private:
    string filename ; 
    DeleteType deletetype;
public:
    FixedRecord(string filename, DeleteType deletetype) {
        this->filename = filename;
        this->deletetype = deletetype;
    }

    vector<Alumno> load(){
        vector<Alumno> alumnos;
        ifstream file(filename, ios::in | ios::binary);
        if(!file.is_open()){
            cout<<"No se puede abrir" <<endl ; 
            exit(1);
        } 
        if(CantRegistros() == 0){cout << "No hay registros" << endl;}
        if(deletetype == MOVE_THE_LAST){
            while(true){
                Alumno alumno;
                file >> alumno;
                if(file.eof()) break;
                alumnos.push_back(alumno);
            }
        }else if(deletetype == FREE_LIST){
            // free list
        }
        file.close();
        return alumnos;
    }
    void add(Alumno record){
        ofstream file(filename, ios::out | ios::app | ios::binary); 
        if(!file.is_open()){
            cout<<"No se puede abrir" <<endl ; 
            exit(1);
        }
        if(deletetype == MOVE_THE_LAST){
            file << record;
        }else if(deletetype == FREE_LIST){
            //free list
        }
        file.close();
    }
    Alumno readRecord(int pos) {
        Alumno al;
        ifstream file(filename, ios::in | ios::binary) ;
        if(!file.is_open()){
            cout<<"No se puede abrir" <<endl ; 
            exit(1);
        }
        if(pos > CantRegistros()){
            cout << "Posicion invalida read record" << endl;
            file.close();
            exit(1);
        }
        if(deletetype == MOVE_THE_LAST){
            file.seekg(pos*(sizeof(Alumno)-1), ios::beg);
            file >> al;
        }else if(deletetype == FREE_LIST){
            // free list
        }
        file.close();

        return al;
    }
    bool deleteRecord(int pos){
        ofstream file(filename, ios::in | ios::out | ios::binary) ; 
        if(!file.is_open()){
            cout<<"No se puede abrir" <<endl ; 
            exit(1);
        }
        if(pos >= CantRegistros()){
            cout << "Posicion invalida delete" << endl;
            file.close();
            return false;
        }
        if(deletetype == MOVE_THE_LAST){
            int lastPos = CantRegistros();
            Alumno lastAlumno = readRecord(lastPos);
            file.seekp(pos*sizeof(Alumno), ios::beg);
            file << lastAlumno;
        }
        else if(deletetype == FREE_LIST){
            // free list
        }
        
        file.close();
        return true;
    }
    int CantRegistros(){
        if(deletetype == MOVE_THE_LAST) {
            ifstream file(filename, ios::in | ios::binary);
            file.seekg(0, ios::end);
            int size = file.tellg();
            file.close();
            return size / sizeof(Alumno);
        } else if(deletetype == FREE_LIST) {
            return 0 ;
        } else {
            return 0; // valor por defecto
        }
    }
    // destructor
    ~FixedRecord(){
        remove(filename.c_str());
    }

};

void Test_MovetheLast(){
    FixedRecord fr("alumnos.dat", MOVE_THE_LAST);

    Alumno a1 = {"1234", "Juan", "Perez", "Sistemas", 5, 200.0};
    Alumno a2 = {"5678", "Maria", "Lopez", "Industrial", 6, 250.0};
    Alumno a3 = {"9101", "Pedro", "Gomez", "Civil", 7, 300.0};
    Alumno a4 = {"1121", "Ana", "Torres", "Mecanica", 8, 350.0};

    //agregando alumnos 
    fr.add(a1);
    fr.add(a2);
    fr.add(a3);
    fr.add(a4);
    //leyendo los alumnos
    cout<< "-------Leyendo los alumnos-------" << endl;
    vector<Alumno> alumnos = fr.load();
    for(Alumno al : alumnos){
        cout << al.codigo << " " << al.nombre << " " << al.apellidos << " " << al.carrera << " " << al.ciclo << " " << al.mensualidad << endl;
    }
    cout << "----------------" << endl;
    cout << "-------leyendo un registro especifico-------" << endl;
    //leer un registro especifico
    Alumno al = fr.readRecord(2);
    cout << al.codigo << " " << al.nombre << " " << al.apellidos << " " << al.carrera << " " << al.ciclo << " " << al.mensualidad << endl;
    

    cout << "----------------" << endl;
    cout << "Eliminando un registro , el primero" << endl;
    //eliminar un registro
    fr.deleteRecord(0);

    cout << "----------------" << endl;
    cout << "------- leyendo los alumnos despues de eliminar -------" << endl;
    //leyendo los alumnos
    alumnos = fr.load();
    for(Alumno al : alumnos){
        cout << al.codigo << " " << al.nombre << " " << al.apellidos << " " << al.carrera << " " << al.ciclo << " " << al.mensualidad << endl;
    }
    cout<< "-----------agregando un nuevo alumno-----------" << endl;
    //agregando un nuevo alumno
    Alumno a5 = {"1314", "Luis", "Garcia", "Sistemas", 5, 200.0};
    fr.add(a5);
    cout << "----------leyendo los alumnos despues de agregar un nuevo alumno----------" << endl;
    //leyendo los alumnos
    alumnos = fr.load();
    for(Alumno al : alumnos){
        cout << al.codigo << " " << al.nombre << " " << al.apellidos << " " << al.carrera << " " << al.ciclo << " " << al.mensualidad << endl;
    } 
}

void Test_FreeList(){
    // free list
}
int main(){
    cout<< "--------------------------------probando movethelast--------------------------------" << endl;
    Test_MovetheLast();
    cout<< "--------------------------------probando freelist--------------------------------" << endl;
    //Test_FreeList();

    return 0; 
}