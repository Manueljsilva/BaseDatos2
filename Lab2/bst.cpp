#include <iostream>
#include <vector>
#include <string>
#include <fstream>
using namespace std;

struct Record
{
  int cod;
  char nombre[12];
  int ciclo;
  char apellido[12];
  long left, right;
  void setData()
  {
    cout << "Codigo:";
    cin >> cod;
    cout << "Nombre: ";
    cin >> nombre;
    cout << "Apellido: ";
    cin >> apellido;
    cout << "Ciclo: ";
    cin >> ciclo;
  }
  void showData() const
  {
    cout << endl << "Codigo: " << cod;
    cout << endl << "Nombre: " << nombre;
    cout << endl << "Apellido: " << apellido;
    cout << endl << "Ciclo : " << ciclo;
    cout << endl << "l: " << left;
    cout << endl << "r: " << right << endl;
  }
};

class BSTFile
{
private:
  string filename;
  long pos_root;

public:
  BSTFile(string filename)
  {
    this->pos_root = 0;
    this->filename = filename;
  }

  Record find(int key)
  {
    return find(pos_root, key);
  }

  void insert(Record record)
  {
    insert(pos_root, record);
  }

  void inorder()
  {
    inorder(pos_root);
  }

  void printAll()
  {
    fstream file(filename, ios::binary | ios::in);
    if (!file.is_open())
      throw runtime_error("Error opening file.");
    Record record;
    while (file.read(reinterpret_cast<char *>(&record), sizeof(Record)))
    {
      record.showData();
    }
    file.close();
  }

private:
  Record find(long pos_node, int key)
  {
    fstream file(filename, std::ios::binary | std::ios::in);
    if (!file.is_open())
      throw std::runtime_error("Error opening file.");
    file.seekg(pos_node * sizeof(Record), ios::beg);
    Record record;
    file.read(reinterpret_cast<char *>(&record), sizeof(Record));
    file.close();
    if (record.cod == key)
    {
      return record;
    }
    else if (record.cod < key && record.right != -1)
    {
      return find(record.right, key);
    }
    else if (record.cod > key && record.left != -1)
    {
      return find(record.left, key);
    }
    else
    {
      throw runtime_error("Not found");
    }
  }

  void insert(long pos_node, Record rec)
  {
    fstream file(filename, ios::binary | ios::in | ios::out);
    if (!file.is_open())
    {
      fstream file(filename, ios::binary | ios::out | ios::app);
      if (!file.is_open())
        throw runtime_error("Error opening file.");
      file.write(reinterpret_cast<const char *>(&rec), sizeof(Record));
    }
    else
    {
      Record record;
      file.seekg(pos_node * sizeof(Record), std::ios::beg);
      file.read(reinterpret_cast<char *>(&record), sizeof(Record));
      if (record.cod == rec.cod)
      {
        file.close();
        return;
      }
      else if (record.cod < rec.cod)
      {
        if (record.right != -1)
        {
          insert(record.right, rec);
        }
        else
        {
          size_t size = file.tellp() / sizeof(Record);
          record.right = size;
          file.seekp(pos_node * sizeof(Record), ios::beg);
          file.write(reinterpret_cast<const char *>(&record), sizeof(Record));
          file.seekp(0, ios::end);
          file.write(reinterpret_cast<const char *>(&rec), sizeof(Record));
        }
      }
      else
      {
        if (record.left != -1)
        {
          insert(record.left, rec);
        }
        else
        {
          size_t size = file.tellp() / sizeof(Record);
          record.left = size;
          file.seekp(pos_node * sizeof(Record), ios::beg);
          file.write(reinterpret_cast<const char *>(&record), sizeof(Record));
          file.seekp(0, ios::end);
          file.write(reinterpret_cast<const char *>(&rec), sizeof(Record));
        }
      }
    }
    file.close();
  }

  void inorder(long pos_node)
  {
    if (pos_node == -1) {
      return;
    }
    fstream file(filename, ios::binary | ios::in);
    if (!file.is_open()) throw std::runtime_error("Error opening file.");
    Record record;
    file.seekg(pos_node * sizeof(Record), ios::beg);
    file.read(reinterpret_cast<char *>(&record), sizeof(Record));
    file.close();
    inorder(record.left);
    record.showData();
    inorder(record.right);
  }
};

int main()
{
  BSTFile file("data.bin");

  Record records[] = {{10, "George", 5, "Smith", -1, -1},
                      {5, "Helen", 6, "Johnson", -1, -1},
                      {3, "Ivy", 2, "Brown", -1, -1},
                      {7, "John", 4, "Davis", -1, -1},
                      {6, "Kevin", 7, "Miller", -1, -1}};

  for (const Record &record : records)
  {
    file.insert(record);
  }

  cout << "All records in the file:" << endl;
  file.printAll();

  cout << endl << "In-order traversal:" << endl;
  file.inorder();

  return 0;
}
