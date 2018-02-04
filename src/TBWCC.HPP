#if !defined(_TBWCC_CLASS_)
#define _TBWCC_CLASS_

#include <owl\owlpch.h>
#include <owl\module.h>

class  TBWCC
 {
public:
  int iStatus;

  TBWCC();
  ~TBWCC();

private:
  TModule *BWCCModule;
  bool bCtl3D;
  HANDLE hMod;
 };

#endif

