#include "windows.h"
#include "memory"
#include "vector"
#include "string"

using namespace std;

interface MyBase // can use "class" or "struct" here too
{
	//public:  // public needed if it's a "class"
		// these counts, and the _ptrDummyMemBlock are only needed for behavior validation
	static int g_nInstances;
	static LONGLONG g_nTotalAllocated;

	virtual string DoSomething() = 0; // pure virtual
	void *_ptrDummyMemBlock;
	int _size;
	MyBase()
	{
		g_nInstances++;
	}
	virtual ~MyBase() // what happens if you remove the "virtual" ?
	{
		g_nInstances--;
	};
	void DoAllocation(int reqSize)
	{
		_ASSERT_EXPR(reqSize > 4, L"too small");
		// put the Free's in the derived classes to demo virtual dtor
		_ptrDummyMemBlock = malloc(reqSize);
		_size = reqSize;
		g_nTotalAllocated += reqSize;
		_ASSERT_EXPR(_ptrDummyMemBlock != nullptr, L"Not enough memory");
		// store the size of the allocation in the allocated block itself
		*(int *)_ptrDummyMemBlock = reqSize;
	}
	void CheckSize()
	{
		_ASSERT_EXPR(_ptrDummyMemBlock != nullptr, "no memory?");
		// get the size of the allocated block from the block
		auto size = *((int *)_ptrDummyMemBlock);
		_ASSERT_EXPR(_size == size, L"sizes don't match");
	}
};

int MyBase::g_nInstances = 0;
LONGLONG MyBase::g_nTotalAllocated = 0;

class MyDerivedA :
	public MyBase
{
public:
	MyDerivedA(int reqSize)
	{
		DoAllocation(reqSize);
	}
	~MyDerivedA()
	{
		_ASSERT_EXPR(_ptrDummyMemBlock != nullptr, L"_p should be non-null");
		CheckSize();
		free(_ptrDummyMemBlock);
		g_nTotalAllocated -= _size;
		_ptrDummyMemBlock = nullptr;
	}
	string DoSomething()
	{
		CheckSize();
		return "MyDerivedA::DoSomething";
	}
};

class MyDerivedB :
	public	MyBase
{
public:
	MyDerivedB(int reqSize)
	{
		DoAllocation(reqSize);
	}
	~MyDerivedB()
	{
		_ASSERT_EXPR(_ptrDummyMemBlock != nullptr, L"_ptrDummyMemBlock should be non-null");
		free(_ptrDummyMemBlock);
		g_nTotalAllocated -= _size;
		_ptrDummyMemBlock = nullptr;
	}
	string DoSomething()
	{
		CheckSize();
		return "MyDerivedB::DoSomething";
	}
};

void DoTestShared_Ptr(int numIter)
{
	// use shared_ptr to be the owner of the object
	vector<shared_ptr<MyBase >> vecSharedPtr;
	for (int i = 0; i < numIter; i++)
	{
		int numA = 0;
		int numB = 0;
		vecSharedPtr.push_back(make_shared<MyDerivedA>(111));
		vecSharedPtr.push_back(make_shared<MyDerivedB>(222));
		_ASSERT_EXPR(MyBase::g_nInstances == 2, L"should have 2 instances");
		for (auto x : vecSharedPtr)
		{
			auto result = x->DoSomething();
			auto xx = dynamic_pointer_cast<MyDerivedA>(x);
			if (xx != nullptr)
			{
				_ASSERT_EXPR(result == "MyDerivedA::DoSomething", L"should be MyDerivedA");
				numA++;
			}
			else
			{
				_ASSERT_EXPR(dynamic_pointer_cast<MyDerivedB>(x) != nullptr, L"should be MyDerivedB");
				_ASSERT_EXPR(result == "MyDerivedB::DoSomething", L"should be MyDerivedB");
				numB++;
			}
		}
		_ASSERT_EXPR(numA == 1 && numB == 1, L"should have 1 of each instance");
		vecSharedPtr.clear();
		_ASSERT_EXPR(MyBase::g_nInstances == 0, L"should have no instances");
		_ASSERT_EXPR(MyBase::g_nTotalAllocated == 0, L"should have none allocated");
	}
}

void DoTestUnique_Ptr(int numIter)
{
	// use unique_ptr to be the sole owner of the object.
	vector<unique_ptr<MyBase >> vecUniquePtr;
	for (int i = 0; i < numIter; i++)
	{
		vecUniquePtr.push_back(make_unique<MyDerivedA>(123));
		vecUniquePtr.push_back(make_unique<MyDerivedB>(456));
		_ASSERT_EXPR(MyBase::g_nInstances == 2, L"should have 2 instances");
		// because we're using unique_ptr, we must iterate using a ref to the object, 
		// rather than a copy
		for (auto &x : vecUniquePtr)
		{
			auto result = x->DoSomething();
			_ASSERT_EXPR(result == "MyDerivedA::DoSomething" ||
				result == "MyDerivedB::DoSomething"
				, L"should have no instances");
			// no dynamice_pointer_cast for unique_ptr
		}
		vecUniquePtr.clear();
		_ASSERT_EXPR(MyBase::g_nInstances == 0, L"should have no instances");
		_ASSERT_EXPR(MyBase::g_nTotalAllocated == 0, L"should have none allocated");
	}
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	int numIter = 10;
	DoTestShared_Ptr(numIter);
	DoTestUnique_Ptr(numIter);

	MessageBoxA(0, "Done", "CppCovariance", 0);
}