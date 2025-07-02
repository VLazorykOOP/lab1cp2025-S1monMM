#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <stdexcept>
#include <algorithm>

using namespace std;

struct DataPoint {
    double x, T, U;
};

class FileOpenException : public runtime_error {
public:
    FileOpenException(const string& msg) : runtime_error("Failed to open file: " + msg) {}
};

class InvalidInputException : public runtime_error {
public:
    InvalidInputException(const string& msg) : runtime_error("Invalid input: " + msg) {}
};

vector<DataPoint> readData(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) throw FileOpenException(filename);

    vector<DataPoint> data;
    double x, T, U;
    while (file >> x >> T >> U) data.push_back({ x, T, U });
    if (data.empty()) throw FileOpenException("File is empty: " + filename);
    return data;
}

double interpolate(double x, const vector<DataPoint>& data, bool isT) {
    for (size_t i = 1; i < data.size(); ++i) {
        if (x >= data[i - 1].x && x <= data[i].x) {
            double x0 = data[i - 1].x, x1 = data[i].x;
            double y0 = isT ? data[i - 1].T : data[i - 1].U;
            double y1 = isT ? data[i].T : data[i].U;
            return y0 + (y1 - y0) * (x - x0) / (x1 - x0);
        }
    }
    return isT ? data.back().T : data.back().U;
}

double T(double x, const vector<DataPoint>& data) {
    return interpolate(x, data, true);
}

double U(double x, const vector<DataPoint>& data) {
    return interpolate(x, data, false);
}

double Srz(double x, double y, double z, const vector<DataPoint>& data) {
    return (x > y) ?
        (T(x, data) + U(z, data) - T(y, data)) :
        (T(y, data) + U(y, data) - U(z, data));
}

double Srs1(double x, double y, const vector<DataPoint>& data) {
    double val = x * x + 2 * y;
    if (val > 1) return Srz(x, y, x, data) + y * log(val);
    return y + Srz(y, x, y, data);
}

double Qrz(double x, double y, const vector<DataPoint>& data) {
    return (abs(x) < 1) ?
        (x * Srs1(x, y, data)) :
        (y * Srs1(x, y, data));
}

double Rrz(double x, double y, double z, const vector<DataPoint>& data) {
    return (x > y) ?
        (x * y * Qrz(y, z, data)) :
        (x * z * Qrz(x, y, data));
}

double Krn(double x, double y, double z, const vector<DataPoint>& data) {
    return 73.1389 * Rrz(x, y, z, data) + 14.838 * Rrz(x - y, z, y, data);
}

double fun1(double x, double y, double z, const vector<DataPoint>& data) {
    return x * Krn(x, y, z, data) + y * Krn(x, z, y, data) - z * Krn(x, z, y, data);
}

double Srs2(double x, double y, double z, const vector<DataPoint>& data) {
    return (z >= y) ?
        (Srz(x, y, z, data) + 1.44 * y * z) :
        (1.44 * y * Srz(z, x, y, data));
}

double Qrz1(double x, double y, const vector<DataPoint>& data) {
    return (abs(x) < 1) ?
        (x * Srs1(x, y, data)) :
        (y * Srs1(x, y, data));
}

double Rrz2(double x, double y, double z, const vector<DataPoint>& data) {
    return (x > y) ?
        (x * y * Qrz1(y, z, data)) :
        (x * z * Qrz1(x, y, data));
}

double Srs3(double x, double y, double z, const vector<DataPoint>& data) {
    return (z > y) ?
        (Srz(x, y, z, data) + y * z) :
        (y + Srz(z, x, y, data));
}

double Qrz2(double x, double y, const vector<DataPoint>& data) {
    return (abs(x) < 1) ?
        (x * Srs2(x, y, y, data)) :
        (y * Srs2(x, y, y, data));
}

double Rrz3(double x, double y, double z, const vector<DataPoint>& data) {
    return (x > y) ?
        (x * y * Qrz2(y, z, data)) :
        (x * z * Qrz2(x, y, data));
}

double Krn2(double x, double y, double z, const vector<DataPoint>& data) {
    return 83.1389 * Rrz3(x, y, z, data) + 4.838 * Rrz3(x, z, y, data);
}

double fun5(double x, double y, double z) {
    return 4.349 * x * x + 23.23 * y - 2.348 * x * y * z;
}

double fun(double x, double y, double z, const vector<DataPoint>& data) {
    if (x * x + 2 * y > 1) return fun1(x, y, z, data);
    if (Rrz2(x, y, z, data) != 0) return Krn(x, y, z, data);
    if (Rrz3(x, y, z, data) != 0) return Krn2(x, y, z, data);
    return fun5(x, y, z);
}

int main() {
    try {
        double x, y, z;
        cout << "Enter x, y, z: ";
        if (!(cin >> x >> y >> z)) throw InvalidInputException("x, y, z must be numeric values.");

        string file;
        if (x > 1) file = "dat_X_1_1.dat";
        else if (x == 1 || x == -1) file = "dat_X_1_00.dat";
        else file = "dat_X_00_1.dat";

        vector<DataPoint> data = readData(file);
        double result = fun(x, y, z, data);
        cout << "fun(" << x << ", " << y << ", " << z << ") = " << result << endl;
    }
    catch (const FileOpenException& e) {
        cerr << "[File Error] " << e.what() << endl;
        return 1;
    }
    catch (const InvalidInputException& e) {
        cerr << "[Input Error] " << e.what() << endl;
        return 1;
    }
    catch (const exception& e) {
        cerr << "[Unexpected Error] " << e.what() << endl;
        return 1;
    }

    return 0;
}
