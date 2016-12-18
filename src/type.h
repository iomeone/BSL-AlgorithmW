#ifndef SU_BOLEYN_BSL_TYPE_H
#define SU_BOLEYN_BSL_TYPE_H

#include <iostream>
#include <memory>
#include <string>
#include <vector>

struct Mono {
  shared_ptr<Mono> par;
  bool is_const;
  string D;
  vector<shared_ptr<Mono>> tau;
  bool is_forall;
  size_t level;
};

struct Poly {
  bool is_mono;
  shared_ptr<Mono> tau;
  shared_ptr<Mono> alpha;
  shared_ptr<Poly> sigma;
};

bool is_c(shared_ptr<Mono> x) { return x->is_const; }
bool is_f(shared_ptr<Mono> x) { return (!x->is_const) && x->is_forall; }
bool is_e(shared_ptr<Mono> x) { return (!x->is_const) && (!x->is_forall); }

shared_ptr<Mono> find(shared_ptr<Mono> x) {
  if (x->par != nullptr) {
    x->par = find(x->par);
    return x->par;
  } else {
    return x;
  }
}

string to_string(shared_ptr<Poly>);

string to_string(shared_ptr<Mono> tau) {
  tau = find(tau);
  if (is_c(tau)) {
    if (tau->D == "->") {
      return "(" + to_string(tau->tau[0]) + ")->(" + to_string(tau->tau[1]) +
             ")";
    } else {
      string ret = tau->D;
      for (auto t : tau->tau) {
        ret += " (" + to_string(t) + ")";
      }
      return ret;
    }
  } else {
    stringstream s;
    for (size_t i = 0; i < tau->level; i++) {
      s << "[";
    }
    s << (is_f(tau) ? "f" : "e") << tau;
    for (size_t i = 0; i < tau->level; i++) {
      s << "]";
    }
    return s.str();
  }
}

string to_string(shared_ptr<Poly> sigma) {
  if (sigma->is_mono) {
    return to_string(sigma->tau);
  } else {
    stringstream s;
    assert(is_f(sigma->alpha));
    s << sigma->alpha;
    return "forall " + s.str() + ". " + to_string(sigma->sigma);
  }
}

shared_ptr<Mono> new_const_var(const string &D) {
  auto t = make_shared<Mono>();
  t->is_const = true;
  t->D = D;
  t->level = 0;
  return t;
}

shared_ptr<Mono> new_forall_var() {
  auto t = make_shared<Mono>();
  t->is_const = false;
  t->is_forall = true;
  t->level = 0;
  return t;
}

shared_ptr<Mono> new_exists_var() {
  auto t = make_shared<Mono>();
  t->is_const = false;
  t->is_forall = false;
  t->level = 0;
  return t;
}

shared_ptr<Poly> new_poly(shared_ptr<Mono> m) {
  auto p = make_shared<Poly>();
  p->is_mono = true;
  p->tau = m;
  return p;
}

shared_ptr<Poly> new_poly(shared_ptr<Mono> alpha, shared_ptr<Poly> sigma) {
  auto p = make_shared<Poly>();
  p->is_mono = false;
  p->alpha = alpha;
  p->sigma = sigma;
  return p;
}

shared_ptr<Mono> get_mono(shared_ptr<Poly> t) {
  while (!t->is_mono) {
    t = t->sigma;
  }
  return t->tau;
}

shared_ptr<Mono> inst(shared_ptr<Mono> tau,
                      map<shared_ptr<Mono>, shared_ptr<Mono>> &m) {
  tau = find(tau);
  if (is_c(tau)) {
    auto t = make_shared<Mono>(*tau);
    for (size_t i = 0; i < tau->tau.size(); i++) {
      t->tau[i] = inst(tau->tau[i], m);
    }
    return t;
  } else {
    if (m.count(tau)) {
      return m[tau];
    } else {
      return tau;
    }
  }
}

shared_ptr<Mono> inst(shared_ptr<Poly> sigma,
                      map<shared_ptr<Mono>, shared_ptr<Mono>> &m) {
  if (sigma->is_mono) {
    return inst(sigma->tau, m);
  } else {
    if (!m.count(find(sigma->alpha))) {
      m[find(sigma->alpha)] = new_forall_var();
    }
    return inst(sigma->sigma, m);
  }
}

shared_ptr<Mono> inst(shared_ptr<Poly> sigma) {
  map<shared_ptr<Mono>, shared_ptr<Mono>> m;
  return inst(sigma, m);
}
shared_ptr<Mono> inst(shared_ptr<Poly> sigma,
                      vector<shared_ptr<Mono>> &exists) {
  map<shared_ptr<Mono>, shared_ptr<Mono>> m;
  for (auto e : exists) {
    m[e] = new_exists_var();
  }
  return inst(sigma, m);
}

void ftv(set<shared_ptr<Mono>> &, shared_ptr<Poly>);

void ftv(set<shared_ptr<Mono>> &f, shared_ptr<Mono> tau) {
  tau = find(tau);
  if (is_c(tau)) {
    for (size_t i = 0; i < tau->tau.size(); i++) {
      ftv(f, tau->tau[i]);
    }
  } else if (is_f(tau)) {
    f.insert(tau);
  }
}

void ftv(set<shared_ptr<Mono>> &f, shared_ptr<Poly> sigma) {
  if (sigma->is_mono) {
    ftv(f, sigma->tau);
  } else {
    ftv(f, sigma->sigma);
    f.erase(sigma->alpha);
  }
}

shared_ptr<Poly> gen(shared_ptr<map<string, shared_ptr<Poly>>> context,
                     shared_ptr<Mono> tau) {
  tau = find(tau);
  set<shared_ptr<Mono>> f;
  for (auto &c : *context) {
    set<shared_ptr<Mono>> fi;
    ftv(fi, c.second);
    f.insert(fi.begin(), fi.end());
  }
  set<shared_ptr<Mono>> fp;
  ftv(fp, tau);
  for (auto i : f) {
    fp.erase(i);
  }
  map<shared_ptr<Mono>, shared_ptr<Mono>> m;
  for (auto f : fp) {
    if (is_f(f)) {
      m[f] = new_forall_var();
    }
  }
  auto g = new_poly(inst(tau, m));
  for (auto f : m) {
    g = new_poly(f.second, g);
  }
  return g;
}

#endif
