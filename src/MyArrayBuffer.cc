#include <stdlib.h>
#include <exception>

#define byte unsigned char

class MyArrayBuffer{
private:
	size_t _byteLength;
	byte* _data;

public:
	MyArrayBuffer(size_t size){
		_byteLength = size;
		_data = (byte*)calloc(_byteLength, sizeof(byte));
	}

	MyArrayBuffer(size_t begin, size_t end, byte* data){
		if(begin > end || !data)
			throw std::exception("Bad arguments");

		_byteLength = end - begin + 1;
		_data = new byte[_byteLength];
		memcpy(_data, data + begin, _byteLength*sizeof(byte));
	}

	size_t getByteLength(){
		return _byteLength;
	}

	const byte* const getData(){
		return _data;
	}

	byte getItem(size_t index){
		if(index >= _byteLength)
			throw std::exception("Out of range");

		return _data[index];
	}

	void setItem(size_t index, byte newValue){
		if(index >= _byteLength)
			throw std::exception("Out of range");

		_data[index] = newValue;
	}

	MyArrayBuffer* slice(size_t begin, size_t end){
		if(end < begin || end - begin + 1 > _byteLength)
			throw std::exception("Bad arguments");

		return new MyArrayBuffer(begin, end, _data);
	}

	~MyArrayBuffer(){
		delete[] _data;
	}
};
