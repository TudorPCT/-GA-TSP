#include <iostream>
#include<random>
#include<vector>
#include<math.h>
#include <algorithm>
#include <fstream>
#include <stdio.h>




double ga(int &popSize, int &generations,int &size, std::vector<std::vector<int>> &degrees)
{
    int k = int(popSize*0.07),pressure = 4;
    int t = 0;
	std::vector<std::vector<int> > population;
	std::vector<int> costs;
    for(int i = 0; i < popSize; i++) {
       population.push_back(generateRandomVector(size));
       costs.push_back(evaluate(population[i],size));
    }
    while(t < generations) {
        t++;
        auto elit = elitism(population,costs,k);
        selection(population, costs, k, pressure, popSize);
        mutate(population);
        crossover(population,popSize);
        for(int i = 0; i < elit.size(); i++)
        {
            population.push_back(elit[i]);
        }
	    for(int i = 0; i < Pt.size(); i++)
	    {
	        costs[i] = evaluate(population[i],size);
	    }
    }
    const auto[minimum, maximum] = std::minmax_element(costs.begin(), costs.end());
    return *minimum;
}

int main()
{
	int size = 5, popSize = 200, generations = 2000;
	std::vector<std::vector<int>> degrees;
	for(int i = 0; i < 5; i++)
		degrees[i].push_back(2);
	degrees[1][4]=degrees[4][1]=1;
	degrees[3][5]=degrees[5][3]=3;
	degrees[4][5]=degrees[5][4]=3;
	auto x = ga(popSize,generations,size,degrees);
}
