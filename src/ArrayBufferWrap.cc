#include <v8.h>
#include "MyArrayBuffer.cc"

using namespace v8;

static Handle<Value> slice(const Arguments&);
static Handle<Value> getByteLength(Local<String>, const AccessorInfo&);
static Handle<Value> getItem(uint32_t index, const AccessorInfo &info);

static Handle<Object> ArrayBufferNewInstance(){
	HandleScope scope;

	Handle<ObjectTemplate> point_templ = ObjectTemplate::New();
	Handle<FunctionTemplate> sliceFunction = FunctionTemplate::New(slice);
	sliceFunction->SetClassName(String::New("slice"));
	point_templ->Set(String::New("slice"), sliceFunction->GetFunction());
	point_templ->SetAccessor(String::New("byteLength"), getByteLength);
	point_templ->SetIndexedPropertyHandler(getItem);
	point_templ->SetInternalFieldCount(1);
	Local<Object> newObj = point_templ->NewInstance();
	 
	return scope.Close(newObj);
}

static Handle<Value> getItem(uint32_t index, const AccessorInfo &info){
	Local<External> wrap = Local<External>::Cast(info.Holder()->GetInternalField(0));
	try{
		int value = static_cast<MyArrayBuffer*>(wrap->Value())->getItem(index);
		return Integer::New(value);
	}catch(std::exception){
		return Undefined();
	}
}

static Handle<Value> ArrayBufferNewObject(const Arguments &args){
	if(args.Length() < 1)
		return ThrowException(Exception::Error(String::New("Constructor does not take 0 arguments")));

	HandleScope scope;

	Handle<Object> newObj = ArrayBufferNewInstance();
	MyArrayBuffer* newArrayBuffer = new MyArrayBuffer(args[0]->Int32Value());
	newObj->SetInternalField(0, External::New(newArrayBuffer));
	newObj->SetIndexedPropertiesToExternalArrayData((void*)newArrayBuffer->getData(), kExternalByteArray, newArrayBuffer->getByteLength());

	return scope.Close(newObj);
}

static Handle<Value> ArrayBufferNewObject(unsigned int size, byte* data){
	HandleScope scope;

	Handle<Object> newObj = ArrayBufferNewInstance();
	MyArrayBuffer* newArrayBuffer = new MyArrayBuffer(0, size-1, data);
	newObj->SetInternalField(0, External::New(newArrayBuffer));
	newObj->SetIndexedPropertiesToExternalArrayData(data, kExternalByteArray, size);

	return scope.Close(newObj);
}

static Handle<Value> getByteLength(Local<String> property, const AccessorInfo &info) {
	Local<External> wrap = Local<External>::Cast(info.Holder()->GetInternalField(0));
	int value = static_cast<MyArrayBuffer*>(wrap->Value())->getByteLength();
	return Integer::New(value);
}

static Handle<Value> slice(const Arguments& args){
	try{
		if(args.Length() < 1)
			return ThrowException(Exception::Error(String::New("slice does not take 0 arguments")));

		HandleScope scope;
	
		Local<External> wrap = Local<External>::Cast(args.This()->GetInternalField(0));
		MyArrayBuffer* buffer = static_cast<MyArrayBuffer*>(wrap->Value());
		size_t sliceBegin = args[0]->Uint32Value();
		size_t sliceEnd = args.Length() > 1 ? args[1]->Uint32Value() : buffer->getByteLength() - 1;
		MyArrayBuffer* newArrayBuffer = buffer->slice(sliceBegin, sliceEnd);
	
	
		Handle<Object> newObj = ArrayBufferNewInstance();
		newObj->SetInternalField(0, External::New(newArrayBuffer));
		newObj->SetIndexedPropertiesToExternalArrayData((void*)newArrayBuffer->getData(), kExternalUnsignedByteArray, newArrayBuffer->getByteLength());

		return scope.Close(newObj);
	}catch(std::exception ex){
		return ThrowException(Exception::RangeError(String::New(ex.what())));
	}
}
