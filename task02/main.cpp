#include <iostream>
#include <fstream>
#include <vector>
#include <omp.h>
using namespace std;

vector<vector<long long>> getMatrWithoutLine(vector<vector<long long>> matr, int line, int column) {
    int n = matr.size();
    vector<vector<long long>> newMatr;
    for (int i = 0; i < n; ++i) {
        if (i == line) continue;
        vector<long long> line;
        for (int j = 0; j < n; ++j) {
            if (j == column) continue;
            line.push_back(matr[i][j]);
        }
        newMatr.push_back(line);
                                                        }
        return newMatr;
}

long long getDet(vector<vector<long long>> matr) {
    if (matr.size() == 1) {
        return matr[0][0];
    }
    if (matr.size() == 2) {
        return matr[0][0] * matr[1][1] - matr[0][1] * matr[1][0];
    }
    int n = matr.size();
    long long det = 0;
    int sign = 1;
    for (int i = 0; i < n; ++i) {
        det += sign * getDet(getMatrWithoutLine(matr, 0, i));
        sign *= -1;
    }
    return det;
}

void getComplementElements(int i, int count, vector<vector<long long>> matr, vector<vector<long long>>& complementMatr, int numOfThread)
{
    for (int k = 0; k < count; ++k) {
        if (i / matr.size() >= matr.size()) {
            return;
        }
        vector<vector<long long>> newMatr;
#pragma omp critical
        {
            cout << "Thread " << numOfThread << " block" << endl;
            newMatr = getMatrWithoutLine(matr, i / matr.size(), i % matr.size());
        }

        long long det = getDet(newMatr);
        det *= ((i / matr.size() + 1) + (i % matr.size() + 1)) % 2 == 0 ? 1 : -1;
#pragma omp critical
        {
            complementMatr[i / matr.size()][i % matr.size()] = det;
        }
        i++;
    }
}

vector<vector<long long>> complementMatrix(vector<vector<long long>> matr, int threadsCount) {
    vector<vector<long long>> newMatr;
    for (int i = 0; i < matr.size(); ++i) {
        vector<long long> newLine;
        for (int j = 0; j < matr.size(); ++j) {
            newLine.push_back(0);
        }
        newMatr.push_back(newLine);
    }
    int elemsInThread = ((long long)(matr.size() * matr.size()) / threadsCount + 1);
    int i = 0;
    int numOfThreads = 0;
    omp_set_num_threads(threadsCount);
#pragma omp parallel
        {
#pragma omp for
            for (int k = 0 ; k < threadsCount; ++k) {
                getComplementElements(k*elemsInThread, elemsInThread, matr, newMatr, ++numOfThreads);
            }
        }
    return newMatr;
}

int main() {
    string inputFile, outputFile;
    int n;
    int threadsCount;
    cout << "Write input file name: ";
    cin >> inputFile;
    cout << "Write output file name: ";
    cin >> outputFile;
    cout << "Write matrix size n: ";
    cin >> n;
    cout << "Write threads count: ";
    cin >> threadsCount;
    ifstream ifs;
    ifs.open(inputFile);
    vector<vector<long long>> matr;
    for (int i = 0; i < n; ++i) {
        vector<long long> line;
        for (int j = 0; j < n; ++j) {
            int elem;
            ifs >> elem;
            line.push_back(elem);
        }
        matr.push_back(line);
    }
    ifs.close();
    auto comp = complementMatrix(matr, threadsCount);
    ofstream ofs(outputFile);
    for (int i = 0; i < comp.size(); ++i) {
        for (int j = 0; j < comp.size(); ++j) {
            ofs << comp[i][j] << " ";
        }
        ofs << endl;
    }
    return 0;
}


