


// Debugging: enable "assert"

#undef assert
#define assert(expr) ((expr) ? (void)0 : assert_failed(__FILE__, __LINE__))
void assert_failed(u8* file, u32 line);

