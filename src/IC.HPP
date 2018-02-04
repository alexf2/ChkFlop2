#if !defined(_IC_)
#define _IC_


template<class T> class TICObject
 {
public:
  TICObject() { pObj_ = 0; }
  TICObject( T pObj ) { pObj_ = pObj; }
  ~TICObject();

  T operator()( void ) const { return pObj_; }
  TICObject& operator=( T );

private:
  T pObj_;
 };

template <class T> TICObject<T>::~TICObject()
 {
        if( pObj_ ) delete pObj_; pObj_ = 0;
 }

template <class T> TICObject<T>& TICObject<T>::operator=( T rNewObj )
 {
		  if( pObj_ ) delete pObj_; pObj_ = rNewObj;
		  return *this;
 }


#endif

