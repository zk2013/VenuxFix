//
// Created by zhengkai on 2017/9/8.
//

// change to android 5.0.2 art

#ifndef VENUXFIX_DEXPOSED_H
#define VENUXFIX_DEXPOSED_H

#define DEXPOSED_VERSION            "51"
#define PLATFORM_SDK_VERSION        9

typedef     unsigned char     u1;
typedef     char                s1;
typedef     short           s2;
typedef int s4;
typedef long long s8;
typedef unsigned int u4;

#define PACKED(x) __attribute__ ((__aligned__(x), __packed__))
#define MANAGED PACKED(4)

typedef void*  HeapReference_XX;

// C++ mirror of java.lang.Object
class MANAGED   Object {
 public:
  // The number of vtable entries in java.lang.Object.
   static size_t kVTableLength;

   HeapReference_XX klass_;
   uint32_t monitor_;
};
size_t Object::kVTableLength = 11;

// C++ mirror of java.lang.Class
class MANAGED Class : public Object {
 public:
// defining class loader, or NULL for the "bootstrap" system loader
    HeapReference_XX class_loader_;
    HeapReference_XX component_type_;
    HeapReference_XX dex_cache_;
    // static, private, and <init> methods
    HeapReference_XX direct_methods_;
    HeapReference_XX ifields_;
    HeapReference_XX iftable_;
    HeapReference_XX   imtable_;
    HeapReference_XX    name_;
};

// C++ mirror of java.lang.String
class MANAGED String: public Object {
public:
	int32_t GetLength() const;
	int32_t GetUtfLength() const;

private:
	// Field order required by test "ValidateFieldOrderOfJavaCppUnionClasses".
	void* array_;

	int32_t count_;

	uint32_t hash_code_;

	int32_t offset_;

	static Class* java_lang_String_;
};

#endif //VENUXFIX_DEXPOSED_H
