#include <iostream>
#include <array>
#include "./include/smallca.h"

int main(int argc, char const *argv[])
{
	SmallCA arr[15];
	arr[0].qID=11;
	arr[1].qID=11;
	arr[2].qID=11;
	arr[3].qID=11;
	arr[4].qID=22;
	arr[5].qID=22;
	arr[6].qID=22;
	arr[7].qID=33;
	arr[8].qID=33;
	arr[9].qID=44;
	arr[10].qID=55;
	arr[11].qID=55;
	arr[12].qID=55;
	arr[13].qID=55;
	arr[14].qID=55;

	for (auto i : arr) // compiler uses type inference to determine the right type
        std::cout << i.qSize << ' ';
    std::cout << '\n';
	for (auto i : arr) // compiler uses type inference to determine the right type
        std::cout << i.qID << ' ';
    std::cout << '\n';
	compute_cluster_size(arr, 15);

	for (auto i : arr) // compiler uses type inference to determine the right type
        std::cout << i.qSize << ' ';
	return 0;
}