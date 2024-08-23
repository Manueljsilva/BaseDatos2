#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
using namespace std;

struct Alumno {
  string Nombre, Apellidos, Carrera;
  float mensualidad;
};

class VariableRecord {
private:
  string fileName;
  vector<Alumno> alumnos;

public:
  // Constructor: recibe el nombre del archivo
  VariableRecord(const string &fileName) : fileName(fileName) {}

  // Función para cargar todos los registros desde el archivo
  vector<Alumno> load() {
    ifstream file(fileName);

    if (!file.is_open()) {
      cout << "No se pudo abrir el archivo" << endl;
      exit(0);
    }

    string line;

    // Ignoro la primera línea con los nombres de los campos
    getline(file, line);

    // Leo cada línea y proceso la información
    while (getline(file, line)) {
      istringstream iss(line); // Trata las cadenas de texto leídas como un flujo de entrada parecido a cin o ifstream
      Alumno alumno;
      string mensualidadStr;

      getline(iss, alumno.Nombre, '|');
      getline(iss, alumno.Apellidos, '|');
      getline(iss, alumno.Carrera, '|');
      getline(iss, mensualidadStr, '|');
      alumno.mensualidad = stof(mensualidadStr);

      alumnos.push_back(alumno);
    }

    file.close();

    return alumnos;
  }

  // Leer cada registro de alumnos
  void read_alumnos() {
    for (auto &alumno : alumnos) {
      cout<< alumno.Nombre << " | " 
          << alumno.Apellidos << " | "
          << alumno.Carrera << " | "
          << alumno.mensualidad << endl;
    }
  }

  // Agregar un nuevo registro al archivo
  void add(Alumno &record) {
    fstream file(fileName, ios::app); // Append

    if (!file.is_open()) {
      cout << "No se pudo abrir el archivo" << endl;
      exit(0);
    }

    file<< "\n" 
        << record.Nombre << "|"
        << record.Apellidos << "|"
        << record.Carrera << "|"
        << record.mensualidad ;

    file.close();
  }

  // Leer un registro en una posición específica
  Alumno readRecord(int pos) {
    fstream file(fileName);

    if (!file.is_open()) {
      cout << "No se pudo abrir el archivo" << endl;
      exit(0);
    }

    string line;
    Alumno alumno;

    // Ignoro la primera línea
    getline(file, line);

    // Leo hasta llegar a la posición "pos"
    for (int i = 0; i <= pos - 1; ++i) getline(file, line);

    istringstream iss(line);
    string mensualidadStr;

    getline(iss, alumno.Nombre, '|');
    getline(iss, alumno.Apellidos, '|');
    getline(iss, alumno.Carrera, '|');
    getline(iss, mensualidadStr, '|');
    alumno.mensualidad = stof(mensualidadStr);

    file.close();

    return alumno;
  }

  // Función para recargar la lista de alumnos después de añadir un nuevo registro
  void reload() {
    alumnos.clear();
    load();
  }
};

int main() {
  VariableRecord vr("alumnos.txt");

  // Test del método load()
  vector<Alumno> alumnos = vr.load();

  // Leer cada registro de alumnos
  vr.read_alumnos();

  // Añadir un nuevo alumno
  Alumno nuevoAlumno = {"Leonard", "Hofstadter Ramirez", "Fisica", 2000.00};
  vr.add(nuevoAlumno);

  // recargar luego de añadir un registro nuevo
  vr.reload();

  cout << endl << "Despues de agregar registro:"<< endl;
  vr.read_alumnos();

  return 0;
}
