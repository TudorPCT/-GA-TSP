#include <iostream>
#include <random>
#include <vector>
#include <math.h>
#include <algorithm>
#include <fstream>
#include <stdio.h>
#include <chrono>
#include <utility>
#include <functional>
#include <numeric>
#include <string>

std::mt19937_64 g_randomGenerator;
float pi = 4 * atan(1);
double mutationChance = 0.01;

struct point {
    double x, y;
};

std::vector<point> graph;

void getData(std::string path)
{
    std::ifstream input_file(path);
    if (!input_file.is_open()) {
        std::cerr << "Could not open the file - " << path << std::endl;
    }

    std::string line;
    while (line != "NODE_COORD_SECTION") {
        std::getline(input_file, line);
    }
    std::getline(input_file, line);
    while (line != "EOF") {
        int i = 0;
        while (line[i] != ' ') i++;
        i++;
        std::string x_str, y_str;

        while (line[i] != ' ') {
            x_str += line[i++];
        }
        point vertex;
        vertex.x = atof(x_str.c_str());

        i++;

        while (i < line.size()) {
            y_str += line[i++];
        }
        vertex.y = atof(y_str.c_str());

        graph.push_back(vertex);

        std::getline(input_file, line);
    }

    input_file.close();
}

template <class T>
void const printVec(const std::vector<T> vec,size_t l , const char* end = "\n"){;}

std::vector<point> generateRandomVector()
{
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(graph.begin(), graph.end(), g);

    return graph;
}

float rand01(int resolution = 10000) 
{
    return g_randomGenerator() % resolution / float(resolution); //basic, could be improved
}

int evaluate(const std::vector<int> &individual, std::vector<std::vector<int> > &len,int size)
{
	int indCost = 0;
	for(int i = 1; i < size; i++)
		indCost += len[individual[i]][individual[i-1]];
	return indCost;	
}

void selection(std::vector<std::vector<int> > &population,std::vector<int> costs, int k, int pressure, int popSize)
{
    const auto [minimum, maximum] = std::minmax_element(costs.begin(), costs.end());
    std::vector<int> F;
    double fs = 0;
    for(int i = 0; i < costs.size(); i++)
    {
        F.push_back(pow(((*maximum - costs[i]) / (*maximum - *minimum + 0.000001) + 0.01),   pressure));
        fs += F[i];
    }
    std::vector<int> pc;
    pc.push_back(F[0]/fs);
    for(int i = 1; i < F.size(); i++)
    {
        pc.push_back(F[i] / fs + pc[i-1]);
    }
    std::vector<std::vector<int> > nextPop;
    for(int i = 0; i < popSize - k; i++)
    {
        float r = rand01();
        bool chosen = false;
        for(int j = 0; j < pc.size(); j++) {
            if (r <= pc[j]) {
                nextPop.push_back(population[j]);
                chosen = true;
                break;
            }
        }
        if(chosen == false)
            nextPop.push_back(population[population.size()-1]);
    }
    population = nextPop;
}

void mutate(std::vector<std::vector<int> > &population)
{
    std::uniform_real_distribution<double> unif(0, 1);
    for (auto& cromozome : population) {
        for (int i = 0; i < cromozome.size(); i++) {
            if (unif(g_randomGenerator) < mutationChance) {
                int id = g_randomGenerator() % cromozome.size();
                std::swap(cromozome[i], cromozome[id]);
            }
        }
    }
}

bool compare(std::pair<int,int> i, std::pair<int,int> j)
{
	return i.second < j.second;
}

std::pair <std::vector<int>, std::vector<int> > cx(const std::vector<int>&c1, const std::vector<int>& c2)
{
    auto d1 = c1, d2 = c2;
    int pos = 1 + g_randomGenerator() % (c1.size() - 3);
    for (int i = pos; i < c1.size(); i++) {
        d1[i] = c2[i];
        d2[i] = c1[i];
    }
    return std::make_pair(d1,d2);
}

void crossover(std::vector<std::vector<int> > &population, int &popSize)
{
    std::vector<std::pair<int, float> > p;
    for (int i = 0; i < population.size(); i++)
        p.push_back(std::make_pair(i, rand01()));
    std::sort(p.begin(), p.end(), compare);
    int i = 0;
    for (i = 0; i < p.size(); i+=2)
    {
        if(i+1 == p.size() || p[i+1].second >= 0.6)
            break;
        auto x = cx(population[p[i].first],population[p[i+1].first]);
        population[p[i].first] = x.first;
        population[p[i+1].first] = x.second;

    }
    if(p[i].second < 0.6)
    {
        float r = rand01();
        if(r >= 0.5) {
            auto x = cx(population[p[i].first], population[p[i + 1].first]);
            population[p[i].first] = x.first;
            population[p[i + 1].first] = x.second;
        }
    }
}

std::vector<std::vector<int> > elitism(const std::vector<std::vector<int> > &population, const std::vector<int> &costs, const int &k)
{
    std::vector<std::pair<int, int> > p;
    std::vector<std::vector<int> > elit;
    for(int i = 0; i < costs.size(); i++)
    {
        p.push_back(std::make_pair(i,costs[i]));
    }
    std::sort(p.begin(), p.end(), compare);
    for(int i = 0; i < k; i++)
    {
        elit.push_back(population[p[i].first]);
    }
    return elit;
}

int ga(int &popSize, int &generations,int &size, std::vector<std::vector<int> > &len)
{
    int k = int(popSize*0.07),pressure = 4;
    int t = 0;
	std::vector<std::vector<int> > population;
	std::vector<int> costs;
    for(int i = 0; i < popSize; i++) {
       population.push_back(generateRandomVector(size));
       costs.push_back(evaluate(population[i],len,size));
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
	    for(int i = 0; i < population.size(); i++)
	    {
	        costs[i] = evaluate(population[i],len,size);
	    }
    }
    const auto[minimum, maximum] = std::minmax_element(costs.begin(), costs.end());
    return *minimum;
}

int main()
{
	int size = 5, popSize = 200, generations = 2000;
	std::vector<std::vector<int>> len;
	for(int i = 0; i < 5; i++)
		len[i].push_back(2);
	len[1][4]=len[4][1]=1;
	len[3][5]=len[5][3]=3;
	len[4][5]=len[5][4]=3;
//	auto x = ga(popSize,generations,size,len);
}
