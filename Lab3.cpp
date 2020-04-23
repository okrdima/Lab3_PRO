#include <mpi.h>
#include <iostream>
#include <string>
#include <ctime>
#include <vector>
#include <cstdlib> 
#include <algorithm>
#include <iterator>
#include <fstream>

using namespace std;

#define sizeOfArray 16
#define sizePart 4
MPI_Status status;
MPI_Request request;


vector<int> randomArray;
vector<int> sizeOfPartsOfArray;
vector<int> sortedArray;
vector<int> tmpBufferSorting;



void bubbleSort(int* a, int n);

void taskFirst(int rank,int size);
void taskSecond(int rank, int firstProcess,int secondProces, MPI_Status status, MPI_Request request);

vector<int> generateRandomArray() {
	vector<int> randArr(sizeOfArray);
	generate(randArr.begin(), randArr.end(), []() { return rand() % 10; });
	return randArr;
}




int main(int argc, char* argv[])
{

	int rank, size;


	srand(time(NULL));


	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	randomArray = generateRandomArray();
	sizeOfPartsOfArray.resize(sizePart);
	sortedArray.resize(sizeOfArray);
	tmpBufferSorting.resize(sizeOfArray / 2);
	//1
	if (size >= 5) {
		if (rank == size-1 ) { cout << "This task 1: " << endl; }
		taskFirst(rank, size);
	}
	//2

	if (size == sizePart)
	{
		if (rank == 4) { cout << "This task 2:" << endl; }
		
		MPI_Scatter(randomArray.data(), sizePart, MPI_INT, sizeOfPartsOfArray.data(), sizePart, MPI_INT, 0, MPI_COMM_WORLD);

		for (int i = 0; i < size; i++) {
			for (int j = i + 1; j < size; j++) {
				taskSecond(rank, i, j,status,request);
			}
		}

		MPI_Gather(sizeOfPartsOfArray.data(), sizePart, MPI_INT, sortedArray.data(), sizePart, MPI_INT, 0, MPI_COMM_WORLD);

		if (rank == 0)
		{
			cout << "Random array:" << endl;
			copy(randomArray.begin(), randomArray.end(), ostream_iterator<int>(std::cout, " "));
			cout << endl << "Sorted array:" << endl;
			copy(sortedArray.begin(), sortedArray.end(), ostream_iterator<int>(std::cout, " "));
		}

	}
	else {
		if(size<5 && rank == 0) { cout << "You need to create 4 processes to enable bubble sort\n"; }
		
	}

	MPI_Finalize();
	return 0;
}

void taskFirst(int rank, int size) {

	int message;
	if (size >= 5) {

		if (rank == size-1) {
			message = size;
		}
		MPI_Bcast(&message, 1, MPI_INT, size-1, MPI_COMM_WORLD);

		if (rank % 4 == 0) {
			cout << "rank  = " << rank << " message = " << message<<endl;
		}
	}


}

void taskSecond(int rank, int firstProcess,int secondProcess,MPI_Status status, MPI_Request request) {

	if (rank == firstProcess)
	{
		MPI_Irecv(tmpBufferSorting.data(), sizePart, MPI_INT, secondProcess, 0, MPI_COMM_WORLD, &request);
		MPI_Wait(&request, &status);
		for (int i = sizePart; i < sizePart * 2; ++i)
		{
			tmpBufferSorting[i] = sizeOfPartsOfArray[i - sizePart];
		}
		bubbleSort(tmpBufferSorting.data(), sizePart * 2);
		for (int i = 0; i < sizePart; ++i)
		{
			sizeOfPartsOfArray[i] = tmpBufferSorting[i];
		}

		MPI_Irsend(tmpBufferSorting.data() + sizePart, sizePart, MPI_INT, secondProcess, 0, MPI_COMM_WORLD, &request);
		MPI_Wait(&request, &status);
	}
	if (rank == secondProcess)
	{
		MPI_Irsend(sizeOfPartsOfArray.data(), sizePart, MPI_INT, firstProcess, 0, MPI_COMM_WORLD, &request);
		MPI_Wait(&request, &status);
		MPI_Irecv(sizeOfPartsOfArray.data(), sizePart, MPI_INT, firstProcess, 0, MPI_COMM_WORLD, &request);
		MPI_Wait(&request, &status);
	}

}

void bubbleSort(int* a, int n)
{
	for (int i = n - 1; i >= 0; i--) {
		for (int j = 0; j < i; j++)
		{
			if (a[j] > a[j + 1]) swap(a[j], a[j + 1]);
		}
	}
}
