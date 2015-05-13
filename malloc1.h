#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>

struct node {
	char _used;					//set to 0 if free and 1 if used
	struct node *_next;			//pointer to the next chunk of memory
	int _size;					//size of the chunk of memory
};


//Initializes the data to be used by the functions
//--------------------------------------------------	
static void* memAddress;	
static struct node *startNode;
static struct node *memWork;
static struct node *memWorkNew;
static bool bInit = false;
static  long PageSize;


//========================================================================================================
//malloc() function to allocate memory with size_t size as a parameter of the amount of memory to reserve
//========================================================================================================

void *malloc(size_t size)

{
  char *ptrChar;
  int iPages;

	// Intialize nodes and memory
	// ----------------------------
	if (bInit == false) {

		// Get the initial memory block
		// ----------------------------
		PageSize =  sysconf (_SC_PAGESIZE);
		startNode = (struct node *)sbrk(PageSize);
		if (startNode == 0) return 0;

		// Initialize the first node
		// --------------------------
		startNode->_size = 4096;
		startNode->_next = 0;
		startNode->_used = 0;
	}

	// Set to intial node
	// ------------------
	memWork = startNode;
	
	// Loops through the list of nodes until the end of memory is reached
	// -----------------------------------------------------------------
	do {

		// if the size of the current chunk of memory is big enough to allocate
		// the requested size of memory and it is unused then allocate memWork for 
		// the memory, creates a new node to point to the leftover memory
		// and returns the address of the allocated memory
		// -----------------------------------------------------------------------
		if((memWork->_size + sizeof(struct node) > size) && (memWork->_used == 0)) {
			
			// sets memAddress to the address of the allocated memory and tags it as used.
			// ---------------------------------------------------------------------------
			memAddress = (char *)memWork + sizeof(struct node);
			memWork->_used = 1;

			// creates new chunk of memory out of the leftover and changes the 
			// pointers to link it into the list
			// ---------------------------------------------------------------
			ptrChar =(char *)memWork + size + sizeof(struct node);
			memWorkNew = (struct node *)ptrChar;		
			memWorkNew->_used = 0;
			memWorkNew->_size = memWork->_size - size - sizeof(struct node);
			memWorkNew->_next = memWork->_next;

			// changes the pointers of memWork to connect the new node
			// --------------------------------------------------------
			memWork->_next = memWorkNew;
			memWork->_size = size + sizeof(struct node);

			return memAddress;
		}

		memWork = memWork->_next;

	} while(memWork->_next != 0);
		
	// if we get to this point the size of memory availible was not big enough
	// and we allocate a bigger block and return its address, returns 0 if the new
	// block is not big enough
	// ---------------------------------------------------------------------------
	iPages = 1;
	if (long (size + sizeof( struct node)) > PageSize) {

		// Check how many pages to fit the request
		// ----------------------------------------
		iPages = size / PageSize + 1;
	}
	memWorkNew = (struct node *)sbrk(iPages * PageSize);	
	if(memWorkNew == 0) return 0;

	// Initialize the new block and connected to the chain
	// ----------------------------------------------------
	memAddress = memWorkNew + sizeof(struct node);
	memWorkNew->_size = (iPages * PageSize) + sizeof(struct node);
	memWorkNew->_next = 0;
	memWork->_next = memWorkNew;
	
	// Call recursively malloc to get the requested memory
	// ----------------------------------------------------
	return (malloc (size));
};

	
//========================================================================================================
// free() function with ptr as a parameter of a pointer to a currently used block of memory
//========================================================================================================

void free (void* ptr)

{
	struct node* memDelete;
	
		memDelete = memWork;

		//loops through the list of memory chunks and sets _used to 0 if ptr matches that address
		//to free the chunk of memory to be allocated
		// --------------------------------------------------------------------------------------
		while(memDelete->_next != 0){
			if((struct node*)ptr - sizeof(struct node) == memDelete){
				memDelete = (struct node*)ptr - sizeof(struct node);
				memDelete->_used = 0;
			}
			memDelete = memDelete->_next;
		}
}

//========================================================================================================
// calloc() fuction to allocate memory for an array taking in a parameter of num_elts for the number
// and elt_size for the size of each element in the array
////========================================================================================================

void *calloc(size_t num_of_elts, size_t elt_size)

{
	void *allocateMem;
	int allocateSize;
		
		// Get the space and clean the memory
		// -----------------------------------
		allocateSize = num_of_elts * elt_size;
		allocateMem = malloc(allocateSize);
		if (allocateMem != 0) memset(allocateMem, 0x00, allocateSize);

	return allocateMem;
}

//========================================================================================================
// realloc() function taking parameters of pointer to a space of memory and size_t size as the number 
// of bytes to change the allocated memory by
//========================================================================================================

void *realloc(void *pointer, size_t size)

{
	void *allocateMem;

		allocateMem = malloc(size);
		if(allocateMem != 0) memcpy(allocateMem, pointer, size);
		return allocateMem;
}

// ===========================================================================================================
// ===========================================================================================================
// ===========================================================================================================
// ===========================================================================================================
