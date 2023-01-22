#pragma once

#include <array>
#include <iomanip>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace BusinessLogic
{

struct BeatsItem
{
    std::string name{};
    std::string aka{};
    double beats{0};
};


// this class is only used to generate a static list of beat division for delay length
class BeatsList
{
  public:
    enum class Modifier
    {
        None,
        Triplet,
        Dotted,
        DoubleDotted,
        Quintuplet
    };


    static int gcd(int a, int b)
    {
        while (b != 0)
        {
            auto t = b;
            b = a % b;
            a = t;
        }
        return a;
    }

    void saveToList(const int beatNumerator, const int beatDenominator, const int akaNumerator,
                    const int akaDenominator, const Modifier& m, const double beats)
    {
        std::stringstream name;
        std::stringstream aka;
        if (beatNumerator < 64)
        {
            if (beatDenominator != 1)
            {
                name << beatNumerator << "/" << beatDenominator;
            }
            else
            {
                name << beatNumerator;
            }
        }
        else
        {
            name << beatNumerator / beatDenominator;
        }
        switch (m)
        {
            case Modifier::Triplet:
                name << " triplet";
                break;
            case Modifier::None:
                break;
            case Modifier::Dotted:
                name << " dot";
                break;
            case Modifier::DoubleDotted:
                name << " doubledot";
                break;
            case Modifier::Quintuplet:
                name << " quintuplet";
                break;
        }
        if (akaNumerator != 0)
        {
            aka << akaNumerator << "/" << akaDenominator;
        }
        m_beatsItems.push_back(BeatsItem{name.str(), aka.str(), beats});
    }

    void generate()
    {
        auto f = [this](int n, int m) -> std::pair<int, int>
        {
            auto d = gcd(n, m);
            return {n / d, m / d};
        };

        std::map<double, int> used{};
        int index = 0;
        int numerator = 1;
        std::array<Modifier, 5> modifiers{Modifier::Triplet, Modifier::None, Modifier::Dotted, Modifier::DoubleDotted,
                                          Modifier::Quintuplet};
        while (numerator != 512)
        {
            for (auto m : modifiers)
            {
                std::pair<int, int> aka{0, 0};
                auto [beatNumerator, beatDenominator] = f(numerator, 64);
                double multiplier = 1.0;
                bool saveValue = true;
                switch (m)
                {
                    case Modifier::Triplet:
                        multiplier = 2.f / 3.f;
                        aka = f(numerator * 2, 64 * 3);
                        break;
                    case Modifier::None:
                        break;
                    case Modifier::Dotted:
                        if (numerator < 2)
                        {
                            saveValue = false;
                        }
                        multiplier = 1.5f;
                        aka = f(numerator * 3 / 2, 64);
                        break;
                    case Modifier::DoubleDotted:
                        multiplier = 1.75f;
                        if (numerator < 4)
                        {
                            saveValue = false;
                        }
                        aka = f(numerator * 5 / 4, 64);
                        break;
                    case Modifier::Quintuplet:
                        multiplier = 0.8f;
                        if (numerator < 4)
                        {
                            saveValue = false;
                        }
                        aka = f(numerator * 4, 64 * 5);
                        break;
                }
                if (saveValue)
                {
                    auto beats =
                        4 * multiplier * static_cast<double>(beatNumerator) / static_cast<double>(beatDenominator);

                    if (used.find(beats) == used.end())
                    {
                        used[beats] = index++;
                        saveToList(beatNumerator, beatDenominator, aka.first, aka.second, m, beats);
                    }
                }
            }

            numerator <<= 1;
        }
        for (int i = 2; i < 16 * 2; ++i)
        {
            auto p = f(i, 16);
            double beats = static_cast<double>(i) / 16.f;
            if (used.find(beats) == used.end())
            {
                used[beats] = index++;
                saveToList(p.first, p.second, 0, 0, Modifier::None, beats);
            }
        }
    }

    void print()
    {
        std::sort(m_beatsItems.begin(), m_beatsItems.end(), [](auto& lhs, auto& rhs) { return lhs.beats < rhs.beats; });
        size_t index = 0;
        for (auto& v : m_beatsItems)
        {
            std::cout << std::setprecision(24) << "{\"" << v.name << "\",\t\"" << v.aka << "\",\t" << v.beats
                      << "}, // " << index << std::endl;
            ++index;
        }
        std::cout << "\n\n\n";
        for (auto& v : m_beatsItems)
        {
            std::cout << "\"" << v.name << "\"," << std::endl;
        }
    }

  private:
    std::vector<BeatsItem> m_beatsItems{};
};
}
