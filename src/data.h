#ifndef SU_BOLEYN_BSL_DATA_H
#define SU_BOLEYN_BSL_DATA_H

#include <memory>
#include <string>
#include <vector>

struct Mono;
struct Poly;

using namespace std;

struct Constructor {
  string name;
  size_t arg;
  shared_ptr<Poly> sig;
  string data_name;
};

struct Data {
  string name;
  size_t arg;
  vector<shared_ptr<Mono>> exists;
  vector<shared_ptr<Constructor>> constructors;
  size_t maxarg;
  size_t to_ptr;
};

#endif
