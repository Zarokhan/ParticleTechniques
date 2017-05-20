#ifndef _DirectX_H_
#define _DirectX_H_

#include <stdexcept>
#define SAFE_RELEASE(x) if( x ) { (x)->Release(); (x) = 0; }
#define SAFE_RELEASE_INPUT(x) if( x ) { (x)->Unacquire(); (x)->Release(); (x) = 0; }
#define SAFE_DELETE(x) if( x ) { delete(x); (x) = 0; }
#define SAFE_DELETE_ARRAY(x) if( x ) { delete[](x); (x) = 0; }
#define ASSERT(r) if (!r) { throw std::invalid_argument( "Error!" ); return false; }		// BOOL
#define FAIL(r) if(FAILED(r)) { throw std::invalid_argument( "Error!" ); return false; }	// HRESULT

#endif