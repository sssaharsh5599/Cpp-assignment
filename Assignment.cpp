
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <memory>
#include <string>
#include <stdexcept>

using namespace std;

// Abstract class
class Advisory {
public:
    virtual void giveAdvice() const = 0;
    virtual ~Advisory() {}
};

// Base class
class Crop {
protected:
    string name;
    double area;
    string season;
    double yieldPerAcre;

    static int count;

    static void validate(double a, double y) {
        if (a <= 0)
            throw invalid_argument("Area must be greater than 0.");

        if (y <= 0)
            throw invalid_argument("Yield per acre must be greater than 0.");
    }

public:
    Crop() : name("Unknown"), area(1),
             season("Unknown"), yieldPerAcre(1) {
        ++count;
    }

    Crop(const string& n, double a, const string& s)
        : name(n), season(s), yieldPerAcre(1) {
        validate(a, 1);
        area = a;
        ++count;
    }

    Crop(const string& n, double a, const string& s, double y)
        : name(n), area(a), season(s), yieldPerAcre(y) {
        validate(a, y);
        ++count;
    }

    Crop(const Crop& other)
        : name(other.name),
          area(other.area),
          season(other.season),
          yieldPerAcre(other.yieldPerAcre) {
        ++count;
    }

    virtual ~Crop() {
        --count;
    }

    static int getCount() {
        return count;
    }

    string getName() const {
        return name;
    }

    virtual double totalYield() const {
        return area * yieldPerAcre;
    }

    virtual void display() const {
        cout << left << setw(14) << name
             << right << setw(8) << fixed << setprecision(2) << area
             << " " << left << setw(8) << season
             << right << setw(8) << yieldPerAcre
             << setw(12) << totalYield();
    }

    virtual void save(ostream& out) const = 0;

    bool operator>(const Crop& other) const {
        return totalYield() > other.totalYield();
    }

    bool operator<(const Crop& other) const {
        return totalYield() < other.totalYield();
    }

    bool operator==(const Crop& other) const {
        return totalYield() == other.totalYield();
    }
};

int Crop::count = 0;

// Derived class 1
class CerealCrop : public Crop, public Advisory {
    string fertilizerType;
    int irrigationFrequency;

public:
    CerealCrop()
        : Crop(), fertilizerType("Urea"), irrigationFrequency(10) {}

    CerealCrop(const string& n, double a, const string& s,
               double y, const string& f, int irr)
        : Crop(n, a, s, y), fertilizerType(f) {

        if (irr <= 0)
            throw invalid_argument(
                "Irrigation frequency must be greater than 0.");

        irrigationFrequency = irr;
    }

    double totalYield() const override {
        return area * yieldPerAcre;
    }

    void display() const override {
        Crop::display();

        cout << " Cereal | Fertilizer: " << fertilizerType
             << ", irrigate every " << irrigationFrequency
             << " days\n";
    }

    void giveAdvice() const override {
        cout << "[" << name
             << "] Sow in well-drained soil, apply "
             << fertilizerType
             << " in split doses, and irrigate every "
             << irrigationFrequency
             << " days. Watch for rust and stem borer.\n";
    }

    void save(ostream& out) const override {
        out << "CEREAL|" << name << '|'
            << area << '|' << season << '|'
            << yieldPerAcre << '|'
            << fertilizerType << '|'
            << irrigationFrequency << '\n';
    }
};

// Derived class 2
class VegetableCrop : public Crop, public Advisory {
    int harvestsPerSeason;
    int irrigationFrequency;

public:
    VegetableCrop()
        : Crop(), harvestsPerSeason(1),
          irrigationFrequency(3) {}

    VegetableCrop(const string& n, double a, const string& s,
                  double y, int harvests, int irr)
        : Crop(n, a, s, y) {

        if (harvests <= 0)
            throw invalid_argument(
                "Harvests per season must be greater than 0.");

        if (irr <= 0)
            throw invalid_argument(
                "Irrigation frequency must be greater than 0.");

        harvestsPerSeason = harvests;
        irrigationFrequency = irr;
    }

    double totalYield() const override {
        return area * yieldPerAcre * harvestsPerSeason;
    }

    void display() const override {
        Crop::display();

        cout << " Vegetable | Harvests: "
             << harvestsPerSeason
             << ", irrigate every " << irrigationFrequency
             << " days\n";
    }

    void giveAdvice() const override {
        cout << "[" << name
             << "] Use mulching to hold moisture, irrigate every "
             << irrigationFrequency
             << " days, and harvest regularly ("
             << harvestsPerSeason
             << " pickings expected). Check for pests weekly.\n";
    }

    void save(ostream& out) const override {
        out << "VEGETABLE|" << name << '|'
            << area << '|' << season << '|'
            << yieldPerAcre << '|'
            << harvestsPerSeason << '|'
            << irrigationFrequency << '\n';
    }
};

// Input functions
string readLine(const string& prompt) {
    string s;

    cout << prompt;
    getline(cin, s);

    if (s.empty())
        throw invalid_argument("Input cannot be empty.");

    return s;
}

double readDouble(const string& prompt) {
    string s = readLine(prompt);

    size_t pos;
    double v;

    try {
        v = stod(s, &pos);
    }
    catch (...) {
        throw invalid_argument(
            "'" + s + "' is not a valid number.");
    }

    if (pos != s.size())
        throw invalid_argument(
            "'" + s + "' is not a valid number.");

    return v;
}

int readInt(const string& prompt) {
    string s = readLine(prompt);

    size_t pos;
    int v;

    try {
        v = stoi(s, &pos);
    }
    catch (...) {
        throw invalid_argument(
            "'" + s + "' is not a valid whole number.");
    }

    if (pos != s.size())
        throw invalid_argument(
            "'" + s + "' is not a valid whole number.");

    return v;
}

// Farm manager class
class Farm {
    vector<unique_ptr<Crop>> crops;
    const string filename = "crops.txt";

public:

    // Add crop
    void addCrop() {
        int type = readInt(
            "Crop type (1 = Cereal, 2 = Vegetable): ");

        if (type != 1 && type != 2)
            throw invalid_argument(
                "Crop type must be 1 or 2.");

        string name = readLine("Name: ");
        double area = readDouble("Area (acres): ");
        string season = readLine(
            "Season (Kharif/Rabi/Zaid): ");
        double y = readDouble(
            "Yield per acre (quintals): ");

        if (type == 1) {
            string fert = readLine("Fertilizer type: ");
            int irr = readInt(
                "Irrigation frequency (days): ");

            crops.push_back(
                make_unique<CerealCrop>(
                    name, area, season, y, fert, irr));
        }
        else {
            int h = readInt("Harvests per season: ");
            int irr = readInt(
                "Irrigation frequency (days): ");

            crops.push_back(
                make_unique<VegetableCrop>(
                    name, area, season, y, h, irr));
        }

        cout << "Crop added successfully.\n";
    }

    // Display all crops
    void displayAll() const {
        if (crops.empty()) {
            cout << "No crops recorded.\n";
            return;
        }

        cout << left << setw(14) << "Name"
             << right << setw(8) << "Area"
             << " " << left << setw(8) << "Season"
             << right << setw(8) << "Y/Acre"
             << setw(12) << "Total" << "\n";

        cout << string(70, '-') << "\n";

        for (const auto& c : crops)
            c->display();

        cout << "Total crops (static count): "
             << Crop::getCount() << "\n";
    }

    // Analyze yield
    void analyze() const {
        if (crops.empty()) {
            cout << "No crops recorded.\n";
            return;
        }

        double sum = 0;

        const Crop* best = crops[0].get();

        for (const auto& c : crops) {
            sum += c->totalYield();

            if (*c > *best)
                best = c.get();
        }

        cout << fixed << setprecision(2);

        cout << "Total farm yield: "
             << sum << " quintals\n";

        cout << "Average yield per crop: "
             << sum / crops.size() << " quintals\n";

        cout << "Highest yielding crop: "
             << best->getName() << "\n";
    }

    // Compare two crops
    void compareTwo() const {
        if (crops.size() < 2) {
            cout << "Need at least 2 crops to compare.\n";
            return;
        }

        int a = readInt(
            "First crop number (1-" +
            to_string(crops.size()) + "): ");

        int b = readInt(
            "Second crop number (1-" +
            to_string(crops.size()) + "): ");

        if (a < 1 || b < 1 ||
            a > (int)crops.size() ||
            b > (int)crops.size()) {

            throw out_of_range(
                "Crop number out of range.");
        }

        const Crop& c1 = *crops[a - 1];
        const Crop& c2 = *crops[b - 1];

        if (c1 > c2)
            cout << c1.getName() << " yields more than "
                 << c2.getName() << ".\n";

        else if (c1 < c2)
            cout << c2.getName() << " yields more than "
                 << c1.getName() << ".\n";

        else
            cout << "Both crops have equal total yield.\n";
    }

    // Farming advice
    void showAdvice() const {
        if (crops.empty()) {
            cout << "No crops recorded.\n";
            return;
        }

        for (const auto& c : crops) {
            const Advisory* adv =
                dynamic_cast<const Advisory*>(c.get());

            if (adv)
                adv->giveAdvice();
        }
    }

    // Save data to file
    void saveToFile() const {
        ofstream out(filename);

        if (!out)
            throw runtime_error(
                "Could not open " + filename + " for writing.");

        for (const auto& c : crops)
            c->save(out);

        if (!out)
            throw runtime_error("Error while writing to file.");

        cout << "Saved " << crops.size()
             << " crop(s) to " << filename << ".\n";
    }

    // Load data from file
    void loadFromFile() {
        ifstream in(filename);

        if (!in)
            throw runtime_error(
                "Could not open " + filename +
                " (save data first).");

        crops.clear();

        string line;

        while (getline(in, line)) {
            if (line.empty())
                continue;

            vector<string> f;
            stringstream ss(line);
            string part;

            while (getline(ss, part, '|'))
                f.push_back(part);

            if (f.size() != 7)
                continue;

            try {
                double area = stod(f[2]);
                double y = stod(f[4]);

                if (f[0] == "CEREAL") {
                    crops.push_back(
                        make_unique<CerealCrop>(
                            f[1], area, f[3], y,
                            f[5], stoi(f[6])));
                }
                else if (f[0] == "VEGETABLE") {
                    crops.push_back(
                        make_unique<VegetableCrop>(
                            f[1], area, f[3], y,
                            stoi(f[5]), stoi(f[6])));
                }

            }
            catch (const exception& e) {
                cout << "Skipped bad record: "
                     << e.what() << "\n";
            }
        }

        cout << "Loaded " << crops.size()
             << " crop(s) from " << filename << ".\n";
    }
};

// Main function
int main() {
    Farm farm;

    int choice = -1;

    while (choice != 0) {

        cout << "\n=== Smart Farm Management System ===\n"
             << "1. Add crop\n"
             << "2. Display all crops\n"
             << "3. Analyze yield\n"
             << "4. Compare two crops\n"
             << "5. Farming advice\n"
             << "6. Save to file\n"
             << "7. Load from file\n"
             << "0. Exit\n";

        try {
            choice = readInt("Choice: ");

            switch (choice) {
                case 1:
                    farm.addCrop();
                    break;

                case 2:
                    farm.displayAll();
                    break;

                case 3:
                    farm.analyze();
                    break;

                case 4:
                    farm.compareTwo();
                    break;

                case 5:
                    farm.showAdvice();
                    break;

                case 6:
                    farm.saveToFile();
                    break;

                case 7:
                    farm.loadFromFile();
                    break;

                case 0:
                    cout << "Goodbye!\n";
                    break;

                default:
                    cout << "Invalid choice.\n";
            }
        }
        catch (const invalid_argument& e) {
            cout << "Invalid input: "
                 << e.what() << "\n";
        }
        catch (const exception& e) {
            cout << "Error: "
                 << e.what() << "\n";
        }

        if (cin.eof())
            break;
    }

    return 0;
}
