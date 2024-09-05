#include <iostream>
#include <cassert>
#include <cstring>
#include <fstream>
#include <sstream>
#include <map>
#include <string>

using namespace std;

struct Record
{
  char codigo[12]; 
  char nombre[12];
  char apellido[12];
  int ciclo;

  Record(){}
  Record(const string& codigo, const string& nombre, const string& apellido, int ciclo){
    strcpy(this->codigo, codigo.c_str()); // se copia el string a un char[]
    strcpy(this->nombre, nombre.c_str());
    strcpy(this->apellido, apellido.c_str());
    this->ciclo = ciclo;
  }

  void showData() {
    cout << "\nCodigo: " << codigo;
    cout << "\nNombre: " << nombre;
    cout << "\nApellido: " << apellido;
    cout << "\nCiclo : " << ciclo;
  }

  bool operator==(const Record& other) const {
    return strcmp(codigo, other.codigo) == 0 &&
           strcmp(nombre, other.nombre) == 0 &&
           strcmp(apellido, other.apellido) == 0 &&
           ciclo == other.ciclo;
  }

  string getKey() {
    return codigo;
  }
};

class RandomFile {
private:
  string fileName;
  string indexName;
  //Agregue un atributo para mantener el indice en ram
  map<string, long> index; 
  
public:
  RandomFile(string _fileName) {
      this->fileName = _fileName + ".dat";
      this->indexName = _fileName + "_ind.dat";
      readIndex();
  }
  
  ~RandomFile(){
      writeIndex();
  }

  /*
  * leer el indice desde disco
  */
  void readIndex(){
    ifstream indexFile(indexName, ios::binary);
    if(!indexFile.is_open()) {
      cout<<"no abre el index File" <<endl;
      exit(1);
    }
    char codigo[12];
    long pos;
    while (indexFile.read((char *)&codigo, 12) && indexFile.read((char *)&pos, sizeof(pos)))
    {
      index[codigo] = pos;
    }
    indexFile.close();
  }

  /*
  * Regresa el indice al disco
  */
  void writeIndex(){
    ofstream indexFile(indexName, ios::binary);
    if (!indexFile.is_open()) {
        cout << "No se puede abrir el archivo de índice para escribir." << endl;
        return;
    }
    for (auto it = index.begin(); it != index.end(); it++){
        // Write the key
        string key = it->first;
        indexFile.write(key.c_str(), 12); // se escribe el codigo del record
        indexFile.write((char*)&it->second, sizeof(it->second)); // se escribe la posicion
    }
    indexFile.close();
  }

  /*
  * Escribe el registro al final del archivo de datos. Se actualiza el indice. 
  */
  void write_record(Record record) {
    ofstream file(fileName, ios::binary | ios::ate | ios::app);
    long pos = file.tellp(); // tellp() regresa la posición actual del puntero de escritura
    file.write((char *)&record, sizeof(record));
    file.close();
    index[record.getKey()] = pos; // se actualiza el indice

    ofstream indexFile(indexName, ios::binary | ios::app);
    indexFile.write((char *)&record.codigo, 12);
    indexFile.write((char *)&pos, sizeof(pos));
    indexFile.close();
  }


  /*
  * Busca un registro que coincida con la key
  */
  Record* find(string key) {
    if(index.find(key) == index.end()){
      cout << "No se encontro el registro" << endl;
      return nullptr;

    }
    ifstream dataFile(fileName, ios::binary);
    dataFile.seekg(index[key]);
    Record* record = new Record;
    dataFile.read((char *)record, sizeof(*record));
    dataFile.close();

    return record;
  }

  /*
  * Muestra todos los registros de acuerdo como fueron insertados en el archivo de datos
  */
  void scanAll() {
    ifstream file(fileName, ios::binary);
    Record record;
    while (file.read((char *)&record, sizeof(record)))
    {
        record.showData();
    }
    file.close();
  }

  /*
  * Muestra todos los registros de acuerdo a como estan ordenados en el indice
  */
  void scanAllByIndex() {
    for(auto &pair : index){
      // creas un puntero a record y le asignas el valor de la busqueda
      Record* record = find(pair.first);
      // muestras los datos
      record->showData();
      delete record;
    }

  }
  map<string , long> getIndex(){
    return index;
  }
  void buildFromCSV(string filename){
    ifstream file(filename);
    if(!file.is_open()){
      cout << "No se puede abrir el archivo CSV" << endl;
      exit(1);
    }
    string line;
    //ignorar la priemra linea
    getline(file, line);
    while(getline(file, line)){
      stringstream stream(line); // se crea un stream con la linea
      string codigo, nombre, apellido, ciclo;

      // lee los datos de la linea separados por coma
      getline(stream, codigo, ',');
      getline(stream, nombre, ',');
      getline(stream, apellido, ',');
      getline(stream, ciclo, ',');

      Record record(codigo, nombre, apellido, stoi(ciclo));
      write_record(record);
    }
    file.close();
  }

};




//--------------- testing ------------------//
void testLeerIndice(RandomFile &rf) {
    assert(!rf.getIndex().empty() && "El índice no debería estar vacío después de leer desde el archivo CSV.");
    cout << "testLeerIndice pasado." << endl;
}

void testBuscarRegistro(RandomFile &rf, Record otro) { //ants habia un record &otro
    Record* result = rf.find(otro.getKey());   
    assert((*result) == otro && "El registro no concuerda con el registrado en BD.");    
    cout << "testBuscarRegistro para el codigo " << otro.getKey() << " pasado." << endl;
}

int main() {
  RandomFile rf("rf_data" );
  cout<<"hasta aca bien"<<endl;
  rf.buildFromCSV("datos.csv");
  cout<<"hasta aca bien 2"<<endl;
  testLeerIndice(rf);
  cout<<"hasta aca bien 3"<<endl;
  testBuscarRegistro(rf, Record("23803540", "Isabel", "Gil", 5));
  testBuscarRegistro(rf, Record("51979300", "Montserrat", "Navarro", 2));
  testBuscarRegistro(rf, Record("66994658", "Marta", "Sanz", 3));
  testBuscarRegistro(rf, Record("21678159", "Vicente", "Garrido", 1));
  testBuscarRegistro(rf, Record("66384772", "Francisca", "Rubio", 10));
  testBuscarRegistro(rf, Record("19425339", "Rosa", "Ramos", 4));    
  cout << "Todos los tests pasaron correctamente." << endl;

  // Agruegue mas casos de prueba.
  
  return 0;
}
