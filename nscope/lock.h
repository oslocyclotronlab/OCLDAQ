#include <string>
#include <cstdio>

class lock
{
public:
  lock(std::string name);
  ~lock(void);
  void ldelete(void);
private:
  FILE *l;
  std::string name_base;
};


