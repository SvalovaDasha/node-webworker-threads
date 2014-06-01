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

		result->SetHiddenValue(String::New("transfer"), Integer::New(1));
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
	for(size_t i=0; i<listLength; i++){
		Handle<Object> sample = list->Get(i)->ToObject();
		for(size_t j=i+1; j<listLength; j++){
			if(sample->Equals(list->Get(j)->ToObject()))
				return j;
		}
	}

	return 0;
}

size_t testOnTransferable(Handle<Object> list){
	if(!list->IsArray())
		return -1;
	size_t listLength = list->Get(String::New("length"))->Uint32Value();
	for(size_t i=0; i<listLength; i++){
		if(getTransferableType(list->Get(i)->ToObject()) == ErrorType)
			return i;
	}

	return -1;
}

bool isArrayBufferInList(const Handle<Object>& arrayBuffer, const std::vector<byte*>& list){
	byte* dataPointer = (byte*)(arrayBuffer->GetIndexedPropertiesExternalArrayData());
	for(size_t i=0, listSize = list.size(); i<listSize; i++){
		if(list[i] == dataPointer){
			return true;
		}
	}

	return false;
}

bool isArrayBuffer(const Handle<Object>& object){
	return object->GetConstructorName()->ToString()->Equals(String::New("ArrayBuffer"));
}

void goDeeper(const Handle<Object>& source, const std::vector<byte*>& transferPointersList, Handle<Object>& sourceCopy){
	Handle<Array> sourcePropertyNames = source->GetPropertyNames();
	size_t sourcePropertyNumber = sourcePropertyNames->Length();
	for(size_t i=0; i<sourcePropertyNumber; i++){
		Handle<String> currentPropertyName = sourcePropertyNames->Get(i)->ToString();
		Handle<Value> currentProperty = source->Get(currentPropertyName);
		Handle<Object> currentPropertyObject = currentProperty->ToObject();
		if(currentPropertyObject->IsNull() || (isArrayBuffer(currentPropertyObject) 
					&& !isArrayBufferInList(currentPropertyObject, transferPointersList))){
			sourceCopy->Set(currentPropertyName, currentProperty);
			continue;
		}

		if(isArrayBuffer(currentPropertyObject)){
			Handle<Object> objectForTransfer = getObjectForTransfer(currentPropertyObject)->ToObject();
			if(objectForTransfer->Has(String::New("transfer"))){
				sourceCopy->Set(currentPropertyName, objectForTransfer);
				currentPropertyObject->TurnOnAccessCheck();
			}
		}else{
			if(currentProperty->IsObject()){
				goDeeper(currentPropertyObject, transferPointersList, currentPropertyObject);
				sourceCopy->Set(currentPropertyName, currentPropertyObject);
			}else
				sourceCopy->Set(currentPropertyName, currentProperty);
		}
	}
}

Handle<Value> getPackedObject(const Handle<Object>& source, const Handle<Object>& transferList){
	HandleScope scope;

	Handle<Object> result = Object::New();
	std::vector<byte*> transferPointers;
	size_t transferListSize = transferList->Get(String::New("length"))->Uint32Value();
	for(size_t i=0; i<transferListSize; i++)
		transferPointers.push_back((byte*)(transferList->Get(i)->ToObject()->GetIndexedPropertiesExternalArrayData()));
	goDeeper(source, transferPointers, result);

	return scope.Close(result);
}