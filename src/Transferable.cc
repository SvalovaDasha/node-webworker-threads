#include <v8.h>

using namespace v8;

#define byte unsigned char

enum TransferableType { ErrorType, ArrayBuffer, CanvasProxy, MessagePort };

static Handle<Value> DataCloneError(Handle<String> message){
	HandleScope scope;
	Handle<Object> result = Object::New();
	result->Set(String::New("message"), message);
	result->Set(String::New("name"), String::New("DataCloneError"));

	return scope.Close(result);
} 

TransferableType getTransferableType(Handle<Object> object){
	Handle<String> constructorName = object->GetConstructorName();
	if(constructorName->Equals(String::New("ArrayBuffer"))
		|| (constructorName->Equals(String::New("Object")) &&  object->HasIndexedPropertiesInExternalArrayData() 
				&& object->Has(String::New("byteLength"))))
		return ArrayBuffer;

	return ErrorType;
}

static Handle<Value> getObjectForTransfer(Handle<Object> source){
	HandleScope scope;
	TransferableType sourceTransType = getTransferableType(source);
	Local<Object> result = Object::New();

	switch(sourceTransType){
	case ArrayBuffer:{
		byte* data = ((byte*)source->GetIndexedPropertiesExternalArrayData());
		int dataLength = source->GetIndexedPropertiesExternalArrayDataLength();

		result->Set(String::New("transfer"), Integer::New(1));
		result->Set(String::New("byteLength"), v8::Uint32::New(dataLength < 0 ? 0 : dataLength));
		result->Set(String::New("dataPointer"), Integer::New((int)data));	
		result->Set(String::New("TransferableType"), v8::Uint32::New(sourceTransType));

		break;
	}
	case CanvasProxy:
		return scope.Close(Exception::TypeError(String::New("Not Implemented")));
		break;
	case MessagePort:
		return scope.Close(Exception::TypeError(String::New("Not Implemented")));
		break;
	default:
		return scope.Close(DataCloneError(String::New("Type is not transferable")));
	}

	return scope.Close(result);
}

size_t testOnUnique(Handle<Object> list){
	if(!list->IsArray())
		return 0;
	size_t listLength = list->Get(String::New("length"))->Uint32Value();
	for(int i=0; i<listLength; i++){
		Handle<Object> sample = list->Get(i)->ToObject();
		for(int j=i+1; j<listLength; j++){
			if(sample->Equals(list->Get(j)->ToObject()))
				return j;
		}
	}

	return 0;
}