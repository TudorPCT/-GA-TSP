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
#include <math.h>
#include <limits>

std::mt19937_64 g_randomGenerator;
float pi = 4 * atan(1);
double mutationChance = 0.1, crossChance = 0.8;

struct point {
    int id;
    double x, y;
};

std::vector<point> graph;

void initRandomGenerator(long int additional = 0) {
    std::mt19937_64 initialiser;
    unsigned long int thisSeed = static_cast<long unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count()) + additional;
    initialiser.seed(thisSeed);
    initialiser.discard(10000);
    g_randomGenerator.seed(initialiser());
}

std::vector<point> getData(std::string path)
{
    std::ifstream input_file(path);
    if (!input_file.is_open()) {
        std::cerr << "Could not open the file - " << path << std::endl;
        exit(0);
    }

    std::string line;
    while (line != "NODE_COORD_SECTION") {
        std::getline(input_file, line);
    }
    std::getline(input_file, line);
    while (line != "EOF") {
        int i = 0;
        std::string id_str;
        point vertex;
        while (line[i] == ' ') i++;
        while (line[i] != ' ')
            id_str += line[i++];
        vertex.id = atoi(id_str.c_str()) - 1;
        std::string x_str, y_str;
        i++;
        while (line[i] == ' ') {
            i++;
        }
        while (line[i] != ' ') {
            x_str += line[i++];
        }
        vertex.x = atof(x_str.c_str());
        i++;

        while (line[i] == ' ') {
            i++;
        }
        while (i < line.size()) {
            y_str += line[i++];
        }
        vertex.y = atof(y_str.c_str());
        graph.push_back(vertex);

        std::getline(input_file, line);
    }

    input_file.close();
    return graph;
}

std::vector<point> generateRandomVector()
{
    /*std::random_device rd;
    std::mt19937 g(rd());*/
    std::shuffle(graph.begin(), graph.end(), g_randomGenerator);

    return graph;
}

float rand01(int resolution = 10000)
{
    return g_randomGenerator() % resolution / float(resolution); //basic, could be improved
}

double evaluate(const std::vector<point>& cromozome)
{
    double cromCost = 0;
    int n = cromozome.size();
    for (int i = 1; i < cromozome.size(); i++)
        cromCost += sqrt(pow(cromozome[i].x - cromozome[i - 1].x, 2) + pow(cromozome[i].y - cromozome[i - 1].y, 2));
    cromCost += sqrt(pow(cromozome[n-1].x - cromozome[0].x, 2) + pow(cromozome[n-1].y - cromozome[0].y, 2));
    return cromCost;
}

void selection(std::vector<std::vector<point> >& population, const std::vector<double>& costs, int pressure, int popSize)
{
    const auto [minimum, maximum] = std::minmax_element(costs.begin(), costs.end());
    std::vector<double> fitness;
    double fs = 0;
    for (int i = 0; i < costs.size(); i++)
    {
        fitness.push_back(pow(((*maximum - costs[i]) / (*maximum - *minimum + 0.000001) + 1), pressure));
        //fitness.push_back(pow(((double)(1 / costs[i])),pressure));
        fs += fitness[i];
    }
    std::vector<double> pc;
    pc.push_back(fitness[0] / fs);
    for (int i = 1; i < fitness.size(); i++)
    {
        pc.push_back(fitness[i] / fs + pc[i - 1]);
    }
    std::vector<std::vector<point> > nextPop;
    for (int i = 0; i < popSize; i++)
    {
        float r = rand01();
        bool chosen = false;
        for (int j = 0; j < pc.size(); j++) {
            if (r <= pc[j]) {
                nextPop.push_back(population[j]);
                chosen = true;
                break;
            }
        }
        if (chosen == false)
            nextPop.push_back(population[population.size() - 1]);
    }
    population = nextPop;
}

int findcity(std::vector<point> &cromozome, point &candidate)
{
	double minim = std::numeric_limits<double>::max();
	int result;
	for(int i = 0; i < cromozome.size(); i++)
	{
		if(cromozome[i].id == candidate.id)
			continue;
		double x = sqrt(pow(cromozome[i].x - candidate.x, 2) + pow(cromozome[i].y - candidate.y, 2));
		if(x < minim){
			minim = x;
		 	result = i;
		}
	}
	return result;
}

std::vector<point> invert(std::vector<point> cromozome)
{
	int i = g_randomGenerator() % cromozome.size();
	int j = g_randomGenerator() % cromozome.size();
	if (i > j)
    std::swap(i, j);
	i++;
	for (; i < j; i++ && j--)
		std::swap(cromozome[i], cromozome[j]);
	return cromozome;
}

std::vector<point> rgibnnm(std::vector<point> cromozome)
{
	int i = g_randomGenerator() % cromozome.size();
	int j = findcity(cromozome, cromozome[i]);
	int z = g_randomGenerator() % 11 - 5;
	if (j + z < 0)
	    j = 0;
	else if (j + z >= cromozome.size())
		j = cromozome.size() - 1;
	else j += z;
	std::swap(cromozome[i], cromozome[j]);
	return cromozome;
}

std::vector<point> slide(std::vector<point> cromozome)
{
	int i = g_randomGenerator() % cromozome.size();
	int j = g_randomGenerator() % cromozome.size();
	if (i > j)
	  	std::swap(i, j);
	for (i++; i < j; i++) 
		std::swap(cromozome[i], cromozome[i + 1]);
	return cromozome;
}

std::vector<point> irgibnnm(std::vector<point> cromozome)
{
	cromozome = invert(cromozome);
	cromozome = rgibnnm(cromozome);
	return cromozome;
}

void mutate(std::vector<std::vector<point> >& population,std::vector<double> costs)
{
	
	/*std::uniform_real_distribution<double> unif(0, 1);
    for (auto& cromozome : population) {
    	for (int i = 0; i < cromozome.size(); i++) {
        	if (unif(g_randomGenerator) < mutationChance) {
            	int id = g_randomGenerator() % cromozome.size();
                std::swap(cromozome[i], cromozome[id]);
             }
         }
	}*/
	
	std::uniform_real_distribution<double> unif(0, 1);
    for (auto& cromozome : population) 
    	if (unif(g_randomGenerator) < mutationChance) {
			auto x = irgibnnm(cromozome);
			auto y = invert(cromozome);
			auto z = slide(cromozome);
			auto xc = evaluate(x);
			auto yc = evaluate(y);
			auto zc = evaluate(z);
			if(xc < yc && xc < zc)
				cromozome = x;
			else if(yc < xc && yc < zc)
				cromozome = y;
			else cromozome = z;
		
	}
}

bool compare(std::pair<int, double> i, std::pair<int, double> j)
{
    return i.second < j.second;
}

std::vector<point> cx(const std::vector<point>& c1, const std::vector<point>& c2)
{
    std::vector<point> cromozome = c1;
    std::vector<int> visited(c1.size(), 0);
    int pos = 1 + g_randomGenerator() % (c1.size() - 3);
    for (int i = 0; i < pos; i++) {
        visited[c1[i].id] = 1;
    }
    int x = 0;
    for (int i = 0; i < c2.size(); i++) {
        if (visited[c2[i].id] == 0)
        {
            cromozome[pos + x] = c2[i];
            x++;
        }
    }
    return cromozome;
}

void crossover(std::vector<std::vector<point> >& population)
{
    std::uniform_real_distribution<double> unif(0, 1);
    std::vector<std::pair<int, double> > p;
    for (int i = 0; i < population.size(); i++)
        p.push_back(std::make_pair(i, unif(g_randomGenerator)));
    std::stable_sort(p.begin(), p.end(), compare);
    int i = 0;
    for (i = 0; i < p.size(); i += 2)
    {
        if (i + 1 == p.size() || p[i + 1].second >= crossChance)
            break;
        auto x = cx(population[p[i].first], population[p[i + 1].first]);
        population.push_back(x);

    }
    if (p[i].second < crossChance)
    {
        float r = unif(g_randomGenerator);
        if (r < 0.5) {
            auto x = cx(population[p[i].first], population[p[i + 1].first]);
            population.push_back(x);
        }
    }
}

std::vector<std::vector<point> > elitism(const std::vector<std::vector<point> >& population, const std::vector<double>& costs, const int& k)
{
    std::vector<std::pair<int, double> > p;
    std::vector<std::vector<point> > elit;
    for (int i = 0; i < costs.size(); i++)
    {
        p.push_back(std::make_pair(i, costs[i]));
    }
    //std::cout << "HERE1\n";

    std::stable_sort(p.begin(), p.end(), compare);
    //std::cout << "HERE2\n";

    for (int i = 0; i < k; i++)
    {
        elit.push_back(population[p[i].first]);
    }
    //std::cout << "HERE3\n";

    return elit;
}

double ga(const int& popSize, const int& generations)
{
	int k = int(popSize * 0.1),pressure = 4;
    int t = 0;
    std::vector<std::vector<point> > population;
    std::vector<double> costs;
    double minimG = std::numeric_limits<double>::max();
    for (int i = 0; i < popSize; i++) {
        population.push_back(generateRandomVector());
        costs.push_back(evaluate(population[i]));
        if (costs[i] < minimG) minimG = costs[i];
    }
    while (t < generations) {
        t++;
        auto elit = elitism(population, costs, k);
        //std::cout << "HERE1\n";
        selection(population, costs, pressure, popSize);
        //std::cout << "HERE2\n";

        mutate(population,costs);
        //std::cout << "HERE3\n";

        crossover(population);
        //std::cout << "HERE4\n";

        for (int i = 0; i < elit.size(); i++)
        {
            population.push_back(elit[i]);
        }
        costs.clear();
        for (int i = 0; i < population.size(); i++)
        {
            costs.push_back(evaluate(population[i]));
            if (costs[i] < minimG) {
                minimG = costs[i];
                //std::cout << t << "-" << minimG << "\n";
            }
        }
        //std::cout << "HERE5\n";

    }
    return minimG;
}

int main(int argc, char* argv[])
{
    auto start = std::chrono::steady_clock::now();
    std::string instance;
    if (argc > 1) {
        instance = argv[1];
    }
    int popSize = 200, generations = 2000;
    getData(instance);
    double x;
	int i = 0;
	while(i < 5)
	{ 
		x += ga(popSize, generations);
		i++;
	}
	std::cout << x / i << "\n";
    //auto x = generateRandomVector();
    auto end = std::chrono::steady_clock::now();
    std::cout << "Elapsed time in miliseconds: " << (double)std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms\n";
    std::cout << "Elapsed time in seconds: " << (double)std::chrono::duration_cast<std::chrono::seconds>(end - start).count() << " sec\n";
    return 0;
}
