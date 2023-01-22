
#include <array>
#include <cmath>
#include <iostream>
#include <limits>
#include <vector>

/*
 * some code for checking with https://godbolt.org/
 */

class Delay
{
  public:
    Delay(size_t MaxBufferSize)
        : m_buffer(MaxBufferSize, 0)
    {
    }

    float nextWithIf(const float in)
    {
        asm("NOP"); // add NOP to have some marker for the highly optimized code
        m_buffer[write++] = in;
        if (write >= m_buffer.size())
        {
            write = 0;
        }
        auto result = m_buffer[read++];
        if (read >= m_buffer.size())
        {
            read = 0;
        }
        return result;
    }

    float nextWithModulo(const float in)
    {
        asm("NOP");
        m_buffer[write++] = in;
        write = write % m_buffer.size();
        auto result = m_buffer[read++];
        read = read % m_buffer.size();
        return result;
    }

    float nextWithBitmask(const float in)
    {
        asm("NOP");
        m_buffer[write++] = in;
        write &= m_buffer.size() - 1;
        auto result = m_buffer[read++];
        read &= m_buffer.size() - 1;
        return result;
    }

    void processBlock(float* inPlace, size_t numSamples)
    {
        for (size_t i = 0; i < numSamples; ++i)
        {
            inPlace[i] = nextWithIf(inPlace[i]);
            ++i;
            inPlace[i] = nextWithIf(inPlace[i]);
            ++i;
            inPlace[i] = nextWithIf(inPlace[i]);
            ++i;
            inPlace[i] = nextWithIf(inPlace[i]);
            ++i;

            inPlace[i] = nextWithModulo(inPlace[i]);
            ++i;
            inPlace[i] = nextWithModulo(inPlace[i]);
            ++i;
            inPlace[i] = nextWithModulo(inPlace[i]);
            ++i;
            inPlace[i] = nextWithModulo(inPlace[i]);
            ++i;
            inPlace[i] = nextWithBitmask(inPlace[i]);
            ++i;
            inPlace[i] = nextWithBitmask(inPlace[i]);
            ++i;
            inPlace[i] = nextWithBitmask(inPlace[i]);
            ++i;
            inPlace[i] = nextWithBitmask(inPlace[i]);
        }
    }

  private:
    std::vector<float> m_buffer;
    size_t write{0};
    size_t read{1};
};

int main(int ac, char* av[])
{
    Delay sut{10000};
    for (size_t i = 0; i < 10; ++i)
    {
        std::array<float, 1024> in;
        in[0] = rand();
        in[1] = rand();
        in[2] = rand();
        sut.processBlock(in.data(), 1024);
        std::cout << in[1022];
        std::cout << in[1023];
        std::cout << in[1024];
        sut.processBlock(in.data(), 1017);
        std::cout << in[1005];
        std::cout << in[1006];
        std::cout << in[1007];
        std::cout << sut.nextWithIf(1.f);
    }
}