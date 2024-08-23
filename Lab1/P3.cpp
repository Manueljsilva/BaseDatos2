#include<iostream>
#include<fstream>
#include<vector>
#include<string>

using namespace std ;


struct Matricula {
    string codigo;
    int ciclo;
    float mensualidad;
    string observaciones;
};

void operator >> (ifstream& stream, Matricula& m){
    int codigoSize, observacionesSize;
    //leer el tamaño y el contenido del string codigo
    stream.read((char*)&codigoSize, sizeof(int));
    m.codigo.resize(codigoSize);
    stream.read(&m.codigo[0], codigoSize);
    //leer los otros campos
    stream.read((char*)&m.ciclo, sizeof(int));
    stream.read((char*)&m.mensualidad, sizeof(float));
    //leer el tamaño y el contenido del string observaciones
    stream.read((char*)&observacionesSize, sizeof(int));
    m.observaciones.resize(observacionesSize);
    stream.read(&m.observaciones[0], observacionesSize);

}

void operator <<(ofstream& stream, Matricula& m){
    int codigoSize = m.codigo.size();
    int observacionesSize = m.observaciones.size();
    //escribir el tamaño y el contenido del string codigo
    stream.write((char*)&codigoSize, sizeof(int));
    stream.write(&m.codigo[0], codigoSize);
    //escribir los otros campos
    stream.write((char*)&m.ciclo, sizeof(int));
    stream.write((char*)&m.mensualidad, sizeof(float));
    //escribir el tamaño y el contenido del string observaciones
    stream.write((char*)&observacionesSize, sizeof(int));
    stream.write(&m.observaciones[0], observacionesSize);
}

class VariableRecord{
private:
    string dataFilename;
    string metaFilename;
public:
    VariableRecord(string dataFilename, string metaFilename): dataFilename(dataFilename), metaFilename(metaFilename){}

    vector<Matricula> load(){
        vector<Matricula> matriculas;
        ifstream datafile(dataFilename, ios::in |ios::binary | ios::app);
        if(!datafile.is_open()){
            cout<<"Error al abrir el archivo"<<endl;
            exit(1);
        }
        while(true){
            Matricula m;
            datafile >> m;
            if(datafile.eof()) break;
            matriculas.push_back(m);
        }
        datafile.close();
        return matriculas;
    }
    void add(Matricula m){
        ofstream datafile(dataFilename, ios::out | ios::binary | ios::app );
        ofstream metafile(metaFilename, ios::binary | ios::app );
        if(!datafile.is_open() || !metafile.is_open()){
            cout<<"Error al abrir el archivo"<<endl;
            exit(1);
        }
        //posicion inicial del registro
        streampos pos = datafile.tellp();
        //escribir el registro
        datafile << m;
        //escribir la posicion del registro en el archivo de metadatos
        metafile.write((char*)&pos, sizeof(streampos));

        datafile.close();
        metafile.close();
    }
    Matricula readRecord(int pos){
        ifstream datafile(dataFilename, ios::in | ios::binary);
        ifstream metafile(metaFilename, ios::in | ios::binary );
        if(!datafile.is_open() || !metafile.is_open()){
            cout<<"Error al abrir el archivo"<<endl;
            exit(1);
        }
        //leer la posicion del registro en el archivo de metadatos
        streampos recordPos;
        metafile.seekg(pos*sizeof(streampos), ios::beg);
        metafile.read((char*)&recordPos, sizeof(streampos));

        //moverse a la posicion del registro en el archivo binario
        datafile.seekg(recordPos, ios::beg);

        //leer el registro
        Matricula m;
        datafile >> m;

        datafile.close();
        metafile.close();
        return m;
    }

};
int main(){
    VariableRecord vr("data.dat", "meta.dat");
    cout << "-------------Cargando registros----------" << endl; 
    Matricula m1 = {"123", 1, 100.0, "observaciones"};
    Matricula m2 = {"456", 2, 200.0, "observaciones"};
    Matricula m3 = {"789", 3, 300.0, "observaciones"};
    vr.add(m1);
    vr.add(m2);
    vr.add(m3);
    vector<Matricula> matriculas = vr.load();
    for(Matricula m: matriculas){
        cout << m.codigo << " " << m.ciclo << " " << m.mensualidad << " " << m.observaciones << endl;
    }
    cout << "-------------Leyendo registros----------" << endl;
    Matricula m = vr.readRecord(1);
    cout << m.codigo << " " << m.ciclo << " " << m.mensualidad << " " << m.observaciones << endl;


    return 0 ; 

}