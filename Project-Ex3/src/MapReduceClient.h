#ifndef MAPREDUCECLIENT_H
#define MAPREDUCECLIENT_H

#include <vector>  //std::vector
#include <utility> //std::pair

// input key and value.
// the key, value for the map function and the MapReduceFramework
class K1 {
public:
    /** We define the destructor as virtual because in case of a pointer pointing to the base class
        (K1 in this case), but the dynamic type is a derived class of it, so without virtual dtr,
        the base class's destructor will be called, resulting in possible memory leak of resources
        managed by the derived class. */
	virtual ~K1() = default;
    /** pure virtual function - the only ones to implement this kind
        of function are the derived classes */
	virtual bool operator<(const K1 &other) const = 0;
};

class V1 {
public:
	virtual ~V1() = default;
};

// intermediate key and value.
// the key, value for the Reduce function created by the Map function
class K2 {
public:
	virtual ~K2() = default;
	virtual bool operator<(const K2 &other) const = 0;
};

class V2 {
public:
	virtual ~V2() = default;
};

// output key and value
// the key, value for the writer mechanism created by the Reduce function
class K3 {
public:
	virtual ~K3() = default;
	virtual bool operator<(const K3 &other) const = 0;
};

class V3 {
public:
	virtual ~V3() = default;
};

typedef std::pair<K1*, V1*> InputPair;
typedef std::pair<K2*, V2*> IntermediatePair;
typedef std::pair<K3*, V3*> OutputPair;

typedef std::vector<InputPair> InputVec;
typedef std::vector<IntermediatePair> IntermediateVec;
typedef std::vector<OutputPair> OutputVec;


class MapReduceClient {
public:
	/** gets a single pair (K1, V1) and calls emit2(K2, V2, context) any
	 *  number of times to output (K2, V2) pairs. */
	virtual void map(const K1* key, const V1* value, void* context) const = 0;

	/** All pairs in the vector are expected to have the same key (but not necessarily the same
        instances of K2).
        This function will produce output pairs and will add them to the framework databases
        using the framework function emit3(K3, V3, context).
        The context argument is provided to allow emit3 to receive information from the function
        that called reduce.
        Pay attention that the map and reduce function are called within the framework and the
        input to these functions is passed by the framework.
	    gets a single K2 key and a vector of all its respective V2 values
	    calls emit3(K3, V3, context) any number of times (usually once)
	    to output (K3, V3) pairs. */
	virtual void reduce(const IntermediateVec* pairs, void* context) const = 0;
};


#endif //MAPREDUCECLIENT_H
