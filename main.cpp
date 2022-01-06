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
double mutationChance = 0.008;

struct point {
    int id;
    double x, y;
};

std::vector<point> graph;

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
        i++;
        vertex.id = atoi(id_str.c_str()) - 1;
        std::string x_str, y_str;
        while (line[i] != ' ') {
            x_str += line[i++];
        }
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
    return graph;
}

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

double evaluate(const std::vector<point>& cromozome)
{
    double cromCost = 0;
    for (int i = 1; i < cromozome.size(); i++)
        cromCost += sqrt(pow(cromozome[i].x - cromozome[i - 1].x, 2) + pow(cromozome[i].y - cromozome[i - 1].y, 2));
    return cromCost;
}

void selection(std::vector<std::vector<point> >& population, const std::vector<double>& costs, int pressure, int popSize)
{
    const auto [minimum, maximum] = std::minmax_element(costs.begin(), costs.end());
    std::vector<double> fitness;
    double fs = 0;
    for (int i = 0; i < costs.size(); i++)
    {
        fitness.push_back(pow(((*maximum - costs[i]) / (*maximum - *minimum + 0.000001) + 0.01), pressure));
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

void mutate(std::vector<std::vector<point> >& population)
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

bool compare(std::pair<int, double> i, std::pair<int, double> j)
{
    return i.second <= j.second;
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

void crossover(std::vector<std::vector<point>>& population)
{
    std::uniform_real_distribution<double> unif(0, 1);
    std::vector<std::pair<int, double>> p;
    for (int i = 0; i < population.size(); i++)
        p.push_back(std::make_pair(i, unif(g_randomGenerator)));
    std::sort(p.begin(), p.end(), compare);
    int i = 0;
    for (i = 0; i < p.size(); i += 2)
    {
        if (i + 1 == p.size() || p[i + 1].second >= 0.8)
            break;
        auto x = cx(population[p[i].first], population[p[i + 1].first]);
        population.push_back(x);

    }
    if (p[i].second < 0.8)
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
    std::sort(p.begin(), p.end(), compare);
    for (int i = 0; i < k; i++)
    {
        elit.push_back(population[p[i].first]);
    }
    return elit;
}

double ga(const int& popSize, const int& generations)
{
    int k = int(popSize * 0.01), pressure = 2;
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
        selection(population, costs, pressure, popSize);
        mutate(population);
        crossover(population);
        for (int i = 0; i < elit.size(); i++)
        {
            population.push_back(elit[i]);
        }
        costs.clear();
        for (int i = 0; i < population.size(); i++)
        {
            costs.push_back(evaluate(population[i]));
            if (costs[i] < minimG) minimG = costs[i];
        }
    }
    return minimG;
}

int main(int argc, char* argv[])
{
    std::string instance;
    if (argc > 1) {
        instance = argv[1];
    }
    int popSize = 200, generations = 2000;
    getData(instance);
    auto x = ga(popSize, generations);
    std::cout << x;
    return 0;
}
