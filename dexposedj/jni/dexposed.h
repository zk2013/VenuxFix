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
	uint32_t sfields_;
	uint32_t super_class_;
	uint32_t verify_error_class_;
	uint32_t virtual_methods_;
	uint32_t vtable_;
	uint32_t access_flags_;
	uint32_t class_size_;
	pid_t clinit_thread_id_;
	int32_t dex_class_def_idx_;
	int32_t dex_type_idx_;
	// Number of instance fields that are object refs.
	uint32_t num_reference_instance_fields_;
	// Number of static fields that are object refs,
	uint32_t num_reference_static_fields_;
	// Total object size; used when allocating storage on gc heap.
	// (For interfaces and abstract classes this will be zero.)
	// See also class_size_.
	uint32_t object_size_;
	// Primitive type value, or Primitive::kPrimNot (0); set for generated primitive classes.
	uint32_t primitive_type_;
	// Bitmap of offsets of ifields.
	uint32_t reference_instance_offsets_;
	// Bitmap of offsets of sfields.
	uint32_t reference_static_offsets_;
	// State of class initialization.
	int32_t status_;
	// TODO: ?
	// initiating class loader list
	// NOTE: for classes with low serialNumber, these are unused, and the
	// values are kept in a table in gDvm.
	// InitiatingLoaderList initiating_loader_list_;
	// The following data exist in real class objects.
	// Embedded Imtable, for class object that's not an interface, fixed size.
	uint32_t embedded_imtable_[0];
	// Embedded Vtable, for class object that's not an interface, variable size.
	uint32_t embedded_vtable_[0];
	// Static fields, variable size.
	uint32_t fields_[0];
	// java.lang.Class
	static void* java_lang_Class_;
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

class ArtField: public Object  {
public:
	uint32_t declaring_class_;
	uint32_t access_flags_;
	uint32_t field_dex_idx_;
	uint32_t offset_;
};

class ArtMethod: public Object {
public:
	// Field order required by test "ValidateFieldOrderOfJavaCppUnionClasses".
	// The class we are a part of
	uint32_t declaring_class_;

	// short cuts to declaring_class_->dex_cache_ member for fast compiled code access
	uint32_t dex_cache_resolved_methods_;

	// short cuts to declaring_class_->dex_cache_ member for fast compiled code access
	uint32_t dex_cache_resolved_types_;

	// short cuts to declaring_class_->dex_cache_ member for fast compiled code access
	uint32_t dex_cache_strings_;

	// Method dispatch from the interpreter invokes this pointer which may cause a bridge into
	// compiled code.
	uint64_t entry_point_from_interpreter_;

	// Pointer to JNI function registered to this method, or a function to resolve the JNI function.
	uint64_t entry_point_from_jni_;

	// Method dispatch from portable compiled code invokes this pointer which may cause bridging into
	// quick compiled code or the interpreter.
#if defined(ART_USE_PORTABLE_COMPILER)
	uint64_t entry_point_from_portable_compiled_code_;
#endif

	// Method dispatch from quick compiled code invokes this pointer which may cause bridging into
	// portable compiled code or the interpreter.
	uint64_t entry_point_from_quick_compiled_code_;

	// Pointer to a data structure created by the compiler and used by the garbage collector to
	// determine which registers hold live references to objects within the heap. Keyed by native PC
	// offsets for the quick compiler and dex PCs for the portable.
	uint64_t gc_map_;

	// Access flags; low 16 bits are defined by spec.
	uint32_t access_flags_;

	/* Dex file fields. The defining dex file is available via declaring_class_->dex_cache_ */

	// Offset to the CodeItem.
	uint32_t dex_code_item_offset_;

	// Index into method_ids of the dex file associated with this method.
	uint32_t dex_method_index_;

	/* End of dex file fields. */

	// Entry within a dispatch table for this method. For static/direct methods the index is into
	// the declaringClass.directMethods, for virtual methods the vtable and for interface methods the
	// ifTable.
	uint32_t method_index_;

	static void* java_lang_reflect_ArtMethod_;
};

#endif //VENUXFIX_DEXPOSED_H
