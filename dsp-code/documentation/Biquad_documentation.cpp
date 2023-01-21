
#include "Biquad.h"
#include "GenerateGnuPlot.h"


#include <numbers>
static constexpr auto c_orchestraTuning = 440.f;
static constexpr auto adjacentSemitoneProportion = 1.059463094359295f;

static float musicalNoteToFrequency(const float note)
{
    return 440.f * std::pow(adjacentSemitoneProportion, note - 69);
}

class GeneratePlot
{
  public:
    GeneratePlot(const std::string& title)
        : plt(true)
    {
        // plt.SetTerminal("dumb size 160,40");
        // plt.SetOutput("biquad.txt");
        plt.SetTerminal("svg size 800,600");
        plt.SetOutput("biquad.svg");
        plt.SetTitle(title);
        plt.GnuplotCommand("set grid xtics, ytics");
        plt.GnuplotCommand("set key inside right bottom");
    }

    void TerminalAndOutput(const std::string& type, const std::string& fileName)
    {
        if (type == "dumb")
        {
            plt.SetTerminal("dumb size 160,40");
            plt.SetOutput(fileName + std::string(".txt"));
        }
        if (type == "svg")
        {
            plt.SetTerminal("svg size 1024,800");
            plt.SetOutput(fileName + std::string(".svg"));
        }
        if (type == "png")
        {
            plt.SetTerminal("pngcairo size 1200,1000");
            plt.SetOutput(fileName + std::string(".png"));
        }
    }


    std::vector<float> addXTicks(size_t FirstOctave, size_t LastOctave, size_t OctaveDivision)
    {
        std::vector<float> frequencies;
        GnuPlot::Plot::Tics xTicks{};
        for (size_t o = FirstOctave; o < LastOctave; ++o)
        {
            auto hz = musicalNoteToFrequency(static_cast<float>(o * 12));
            if (hz < 1000)
            {
                std::stringstream hzString;
                hzString << static_cast<int>(hz) << "";
                xTicks.push_back(std::make_pair<std::string, double>(hzString.str(), static_cast<double>(hz)));
            }
            else
            {
                std::stringstream hzString;

                hzString << round(hz / 100.f) / 10.f << "k";
                xTicks.push_back(std::make_pair<std::string, double>(hzString.str(), static_cast<double>(hz)));
            }

            for (size_t i = 0; i < OctaveDivision; ++i)
            {
                float d = 12.f * static_cast<float>(i) / static_cast<float>(OctaveDivision);
                auto hz = musicalNoteToFrequency(static_cast<float>(o * 12 + d));
                frequencies.push_back(hz);
            }
        }
        plt.GnuplotCommand("set logscale x");
        plt.SetXLabel("Hz");
        plt.SetXTics(xTicks);

        return frequencies;
    }

    void addYTicks(int dbLow, int dbHigh)
    {
        GnuPlot::Plot::Tics yTicks{};
        for (int i = dbLow; i <= dbHigh; i += 6)
        {
            std::stringstream dbString;
            dbString << (i > 0 ? "+" : "") << i;
            yTicks.push_back(std::make_pair<std::string, double>(dbString.str(), static_cast<double>(i)));
        }
        plt.SetYLabel("dB");
        plt.SetYRange(-60.0, 6.0);
        plt.SetYTics(yTicks);
    }


    GnuPlot::Plot plt;
};


int main(int ac, char* av[])
{
    DSP::ChebyshevBiquad sut;
    constexpr size_t FirstOctave = 2;
    constexpr size_t LastOctave = 10;
    constexpr size_t OctaveDivision = 36;

    GeneratePlot plotThis("Chby Type1 High pass");
    plotThis.TerminalAndOutput("png", "cheby-biquad");
    auto frequencies = plotThis.addXTicks(FirstOctave, LastOctave, OctaveDivision);
    plotThis.addYTicks(-60, 12);
    std::vector<std::vector<float>> x;
    std::vector<std::vector<float>> y;
    auto drawState = plotThis.plt.StartDraw2D<std::vector<float>::const_iterator>();

    for (size_t order = 2, index = 0; order <= 10; order += 2, ++index)
    {
        x.push_back(std::vector<float>());
        y.push_back(std::vector<float>());
        sut.setSampleRate(48000.f);
        sut.computeType1(order, 1000.f, 3, false);
        for (auto& hz : frequencies)
        {
            auto magnitudeDb = sut.getMagnitudeInDb(hz);
            x[index].push_back(hz);
            y[index].push_back(magnitudeDb);
        }
        plotThis.plt.AddDrawing(drawState, GnuPlot::Lines(x[index].begin(), x[index].end(), y[index].begin(),
                                                          std::to_string(order), "lw 2"));
    }
    plotThis.plt.EndDraw2D(drawState);
    plotThis.plt.Flush();
    return 0;
}