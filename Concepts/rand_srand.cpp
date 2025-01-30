#include <iostream>
#include <cstdlib> // for rand() and srand()
#include <ctime>   // for time()

int main() {
    int n;
    std::cin>>n;
    
    // Seed the random number generator with the current time
    srand(time(0));

    // Generate and print 5 random numbers
    for (int i = 0; i < n; i++) {
        int randomNumber = rand();
        std::cout << randomNumber << std::endl;
    }

    return 0;
}