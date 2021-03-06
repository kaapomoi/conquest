#pragma once
#include <cstdlib>
#include <string>
#include <iostream>
#include <stdarg.h> 
#include <vector>
#include <fstream>
#include <deque>
#include <util/random.hpp>
#include <SDL/SDL.h>
#include <util/vecs.h>
#include <sstream>


namespace k2d
{
    static float floats[20][20] = { 
        0.f,1.f,2.f,3.f,4.f,5.f,6.f,7.f,8.f,9.f,10.f,11.f,12.f,13.f,14.f,15.f,16.f,17.f,18.f,19.f,
        1.f,1.41421f,2.23607f,3.16228f,4.12311f,5.09902f,6.08276f,7.07107f,8.06226f,9.05539f,10.0499f,11.0454f,12.0416f,13.0384f,14.0357f,15.0333f,16.0312f,17.0294f,18.0278f,19.0263f,
        2.f,2.23607f,2.82843f,3.60555f,4.47214f,5.38516f,6.32456f,7.28011f,8.24621f,9.21954f,10.198f,11.1803f,12.1655f,13.1529f,14.1421f,15.1327f,16.1245f,17.1172f,18.1108f,19.105f,
        3.f,3.16228f,3.60555f,4.24264f,5.f,5.83095f,6.7082f,7.61577f,8.544f,9.48683f,10.4403f,11.4018f,12.3693f,13.3417f,14.3178f,15.2971f,16.2788f,17.2627f,18.2483f,19.2354f,
        4.f,4.12311f,4.47214f,5.f,5.65685f,6.40312f,7.2111f,8.06226f,8.94427f,9.84886f,10.7703f,11.7047f,12.6491f,13.6015f,14.5602f,15.5242f,16.4924f,17.4642f,18.4391f,19.4165f,
        5.f,5.09902f,5.38516f,5.83095f,6.40312f,7.07107f,7.81025f,8.60233f,9.43398f,10.2956f,11.1803f,12.083f,13.f,13.9284f,14.8661f,15.8114f,16.7631f,17.72f,18.6815f,19.6469f,
        6.f,6.08276f,6.32456f,6.7082f,7.2111f,7.81025f,8.48528f,9.21954f,10.f,10.8167f,11.6619f,12.53f,13.4164f,14.3178f,15.2315f,16.1555f,17.088f,18.0278f,18.9737f,19.9249f,
        7.f,7.07107f,7.28011f,7.61577f,8.06226f,8.60233f,9.21954f,9.8995f,10.6301f,11.4018f,12.2066f,13.0384f,13.8924f,14.7648f,15.6525f,16.5529f,17.4642f,18.3848f,19.3132f,20.2485f,
        8.f,8.06226f,8.24621f,8.544f,8.94427f,9.43398f,10.f,10.6301f,11.3137f,12.0416f,12.8062f,13.6015f,14.4222f,15.2643f,16.1245f,17.f,17.8885f,18.7883f,19.6977f,20.6155f,
        9.f,9.05539f,9.21954f,9.48683f,9.84886f,10.2956f,10.8167f,11.4018f,12.0416f,12.7279f,13.4536f,14.2127f,15.f,15.8114f,16.6433f,17.4929f,18.3576f,19.2354f,20.1246f,21.0238f,
        10.f,10.0499f,10.198f,10.4403f,10.7703f,11.1803f,11.6619f,12.2066f,12.8062f,13.4536f,14.1421f,14.8661f,15.6205f,16.4012f,17.2047f,18.0278f,18.868f,19.7231f,20.5913f,21.4709f,
        11.f,11.0454f,11.1803f,11.4018f,11.7047f,12.083f,12.53f,13.0384f,13.6015f,14.2127f,14.8661f,15.5563f,16.2788f,17.0294f,17.8045f,18.6011f,19.4165f,20.2485f,21.095f,21.9545f,
        12.f,12.0416f,12.1655f,12.3693f,12.6491f,13.f,13.4164f,13.8924f,14.4222f,15.f,15.6205f,16.2788f,16.9706f,17.6918f,18.4391f,19.2094f,20.f,20.8087f,21.6333f,22.4722f,
        13.f,13.0384f,13.1529f,13.3417f,13.6015f,13.9284f,14.3178f,14.7648f,15.2643f,15.8114f,16.4012f,17.0294f,17.6918f,18.3848f,19.105f,19.8494f,20.6155f,21.4009f,22.2036f,23.0217f,
        14.f,14.0357f,14.1421f,14.3178f,14.5602f,14.8661f,15.2315f,15.6525f,16.1245f,16.6433f,17.2047f,17.8045f,18.4391f,19.105f,19.799f,20.5183f,21.2603f,22.0227f,22.8035f,23.6008f,
        15.f,15.0333f,15.1327f,15.2971f,15.5242f,15.8114f,16.1555f,16.5529f,17.f,17.4929f,18.0278f,18.6011f,19.2094f,19.8494f,20.5183f,21.2132f,21.9317f,22.6716f,23.4307f,24.2074f,
        16.f,16.0312f,16.1245f,16.2788f,16.4924f,16.7631f,17.088f,17.4642f,17.8885f,18.3576f,18.868f,19.4165f,20.f,20.6155f,21.2603f,21.9317f,22.6274f,23.3452f,24.0832f,24.8395f,
        17.f,17.0294f,17.1172f,17.2627f,17.4642f,17.72f,18.0278f,18.3848f,18.7883f,19.2354f,19.7231f,20.2485f,20.8087f,21.4009f,22.0227f,22.6716f,23.3452f,24.0416f,24.7588f,25.4951f,
        18.f,18.0278f,18.1108f,18.2483f,18.4391f,18.6815f,18.9737f,19.3132f,19.6977f,20.1246f,20.5913f,21.095f,21.6333f,22.2036f,22.8035f,23.4307f,24.0832f,24.7588f,25.4558f,26.1725f,
        19.f,19.0263f,19.105f,19.2354f,19.4165f,19.6469f,19.9249f,20.2485f,20.6155f,21.0238f,21.4709f,21.9545f,22.4722f,23.0217f,23.6008f,24.2074f,24.8395f,25.4951f,26.1725f,26.8701f
    };

	extern void KUSI_ERROR(std::string _error_string);

	extern void KUSI_DEBUG(const char* fmt, ...);

	extern bool ReadFileToBuffer(std::string _file, std::vector<unsigned char>& _buffer);

    extern float sqrt(int x, int y);

	extern float DegToRad(float _deg);

    extern double map_to_range(double val, double in_min, double in_max, double out_min, double out_max);

    template<typename T>
    void clamp(T& in, T min, T max) {
        if(in <= min)
        {
            in = min;
        }
        if (in >= max)
        {
            in = max;
        }
    }

   /* extern void clamp(double& in, double min, double max);

    extern void clamp(float& in, float min, float max);

    extern void clamp(int& in, int min, int max);*/

    template <typename Type>
    void CallDestructor(void* ptr)
    {
        Type* p = (Type*)ptr;
        p->~Type();
    }

    template <typename T>
    std::string to_string_p(const T a_value, const int n = 6)
    {
        std::ostringstream out;
        out.precision(n);
        out << std::fixed << a_value;
        return out.str();
    }

}