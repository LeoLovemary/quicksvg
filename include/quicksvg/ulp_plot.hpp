#ifndef QUICKSVG_ULP_PLOT_HPP
#define QUICKSVG_ULP_PLOT_HPP
#include "detail/generic_svg_functionality.hpp"
#include <algorithm>
#include <iomanip>
#include <cassert>
#include <vector>
#include <utility>
#include <fstream>
#include <string>
#include <list>
#include <random>
#if defined __has_include
#  if __has_include (<boost/math/tools/condition_numbers.hpp>)
#    include <boost/math/tools/condition_numbers.hpp>
#    include <boost/random/uniform_real_distribution.hpp>
#  else
#    error "This library requires boost math 1.70 or above"
#  endif
#endif


// Design of this function comes from:
// https://blogs.mathworks.com/cleve/2017/01/23/ulps-plots-reveal-math-function-accurary/

// The envelope is the condition number of function evaluation.

namespace quicksvg {

template<class F, typename PreciseReal, typename CoarseReal>
class ulp_plot {
public:
    ulp_plot(F hi_acc_impl, CoarseReal a, CoarseReal b,
             bool perturb_abscissas = true, size_t samples = 10000, int random_seed = -1)
    {
        static_assert(sizeof(PreciseReal) >= sizeof(CoarseReal), "PreciseReal must have larger size than CoarseReal");
        if (samples < 10)
        {
            throw std::domain_error("Must have at least 10 samples, samples = " + std::to_string(samples));
        }
        if (b <= a)
        {
            throw std::domain_error("On interval [a,b], b > a is required.");
        }
        a_ = a;
        b_ = b;

        std::mt19937_64 gen;
        if (random_seed == -1)
        {
            std::random_device rd;
            gen.seed(rd());
        }
        // Boost's uniform_real_distribution can generate quad and multiprecision random numbers; std's cannot:
        boost::random::uniform_real_distribution<PreciseReal> dis(a, b);
        precise_abscissas_.resize(samples);
        coarse_abscissas_.resize(samples);

        if (perturb_abscissas)
        {
            for(size_t i = 0; i < samples; ++i)
            {
                precise_abscissas_[i] = dis(gen);
            }
            std::sort(precise_abscissas_.begin(), precise_abscissas_.end());
            for (size_t i = 0; i < samples; ++i)
            {
                coarse_abscissas_[i] = static_cast<CoarseReal>(precise_abscissas_[i]);
            }
        }
        else
        {
            for(size_t i = 0; i < samples; ++i)
            {
                coarse_abscissas_[i] = static_cast<CoarseReal>(dis(gen));
            }
            std::sort(coarse_abscissas_.begin(), coarse_abscissas_.end());
            for (size_t i = 0; i < samples; ++i)
            {
                precise_abscissas_[i] = coarse_abscissas_[i];
            }
        }

        precise_ordinates_.resize(samples);
        for (size_t i = 0; i < samples; ++i)
        {
            precise_ordinates_[i] = hi_acc_impl(precise_abscissas_[i]);
        }

        cond_.resize(samples, std::numeric_limits<PreciseReal>::quiet_NaN());
        for (size_t i = 0 ; i < samples; ++i)
        {
            PreciseReal y = precise_ordinates_[i];
            if (y != 0)
            {
                cond_[i] = boost::math::tools::evaluation_condition_number(hi_acc_impl, precise_abscissas_[i]);
                // Half-ULP accuracy is the correctly rounded result, so make sure the envelop doesn't go below this:
                if (cond_[i] < 0.5)
                {
                    cond_[i] = 0.5;
                }
            }
            // else leave it as nan.
        }

    }

    template<class G>
    void add_fn(G g, std::string const & color = "steelblue")
    {
        using std::abs;
        size_t samples = precise_abscissas_.size();
        std::vector<CoarseReal> ulps(samples);
        for (size_t i = 0; i < samples; ++i)
        {
            PreciseReal y_hi_acc = precise_ordinates_[i];
            PreciseReal y_lo_acc = g(coarse_abscissas_[i]);
            PreciseReal absy = abs(y_hi_acc);
            PreciseReal dist = nextafter(static_cast<CoarseReal>(absy), std::numeric_limits<CoarseReal>::max()) - static_cast<CoarseReal>(absy);
            ulps[i] = static_cast<CoarseReal>((y_lo_acc - y_hi_acc)/dist);
        }
        ulp_list_.emplace_back(ulps);
        colors_.emplace_back(color);
        return;
    }

    void write(std::string const & filename, int clip = -1, bool ulp_envelope = true, std::string const & title = "", int width = 1100,
               int horizontal_lines = 8, int vertical_lines = 10)
    {
        using std::abs;
        using std::floor;
        using std::isnan;
        if (ulp_list_.size() == 0)
        {
            throw std::domain_error("No functions added for comparison.");
        }
        if (width <= 1)
        {
            throw std::domain_error("Width = " + std::to_string(width) + ", which is too small.");
        }

        PreciseReal worst_ulp_distance = 0;
        PreciseReal min_y = std::numeric_limits<PreciseReal>::max();
        PreciseReal max_y = std::numeric_limits<PreciseReal>::lowest();
        for (auto const & ulp_vec : ulp_list_)
        {
            for (auto const & ulp : ulp_vec)
            {
                if (abs(ulp) > worst_ulp_distance)
                {
                    worst_ulp_distance = abs(ulp);
                }
                if (ulp < min_y)
                {
                    min_y = ulp;
                }
                if (ulp > max_y)
                {
                    max_y = ulp;
                }
            }
        }

        if (clip > 0)
        {
            if (max_y > clip)
            {
                max_y = clip;
            }
            if (min_y < -clip)
            {
                min_y = -clip;
            }
        }

        int height = floor(double(width)/1.61803);
        int margin_top = 40;
        int margin_left = 25;
        if (title.size() == 0)
        {
            margin_top = 10;
            margin_left = 15;
        }
        int margin_bottom = 20;
        int margin_right = 20;
        int graph_height = height - margin_bottom - margin_top;
        int graph_width = width - margin_left - margin_right;

        // Maps [a,b] to [0, graph_width]
        auto x_scale = [&](CoarseReal x)->CoarseReal
        {
            return ((x-a_)/(b_ - a_))*static_cast<CoarseReal>(graph_width);
        };

        auto y_scale = [&](PreciseReal y)->PreciseReal
        {
            return ((max_y - y)/(max_y - min_y) )*static_cast<PreciseReal>(graph_height);
        };

        std::ofstream fs;
        fs.open(filename);
        fs << "<?xml version=\"1.0\" encoding='UTF-8' ?>\n"
           << "<svg xmlns='http://www.w3.org/2000/svg' width='"
           << width << "' height='"
           << height << "'>\n"
           << "<style>svg { background-color: black; }\n"
           << "</style>\n";
        if (title.size() > 0)
        {
            fs << "<text x='" << floor(width/2)
               << "' y='" << floor(margin_top/2)
               << "' font-family='Palatino' font-size='25' fill='white'  alignment-baseline='middle' text-anchor='middle'>"
               << title
               << "</text>\n";
        }

        // Construct SVG group to simplify the calculations slightly:
        fs << "<g transform='translate(" << margin_left << ", " << margin_top << ")'>\n";
            // y-axis:
        fs  << "<line x1='0' y1='0' x2='0' y2='" << graph_height
            << "' stroke='gray' stroke-width='1'/>\n";
        PreciseReal x_axis_loc = y_scale(static_cast<PreciseReal>(0));
        fs << "<line x1='0' y1='" << x_axis_loc
            << "' x2='" << graph_width << "' y2='" << x_axis_loc
            << "' stroke='gray' stroke-width='1'/>\n";

        if (worst_ulp_distance > 3)
        {
            detail::write_gridlines(fs, horizontal_lines, vertical_lines, x_scale, y_scale, a_, b_,
                                    static_cast<CoarseReal>(min_y), static_cast<CoarseReal>(max_y), graph_width, graph_height, margin_left);
        }
        else
        {
            std::vector<double> ys{-3.0, -2.5, -2.0, -1.5, -1.0, -0.5, 0.5, 1.0, 1.5, 2.0, 2.5, 3.0};
            for (size_t i = 0; i < ys.size(); ++i)
            {
                if (min_y <= ys[i] && ys[i] <= max_y)
                {
                    PreciseReal y_cord_dataspace = ys[i];
                    PreciseReal y = y_scale(y_cord_dataspace);
                    fs << "<line x1='0' y1='" << y << "' x2='" << graph_width
                       << "' y2='" << y
                       << "' stroke='gray' stroke-width='1' opacity='0.5' stroke-dasharray='4' />\n";

                    fs << "<text x='" <<  -margin_left/2 << "' y='" << y - 3
                       << "' font-family='times' font-size='10' fill='white' transform='rotate(-90 "
                       << -margin_left/2 + 11 << " " << y + 5 << ")'>"
                       <<  std::setprecision(4) << y_cord_dataspace << "</text>\n";
                }
            }
            for (int i = 1; i <= vertical_lines; ++i)
            {
                CoarseReal x_cord_dataspace = a_ +  ((b_ - a_)*i)/vertical_lines;
                CoarseReal x = x_scale(x_cord_dataspace);
                fs << "<line x1='" << x << "' y1='0' x2='" << x
                   << "' y2='" << graph_height
                   << "' stroke='gray' stroke-width='1' opacity='0.5' stroke-dasharray='4' />\n";

                fs << "<text x='" <<  x - 10  << "' y='" << graph_height + 10
                   << "' font-family='times' font-size='10' fill='white'>"
                   << std::setprecision(4) << x_cord_dataspace << "</text>\n";
            }
        }

        int color_idx = 0;
        for (auto const & ulp : ulp_list_)
        {
            std::string color = colors_[color_idx++];
            for (size_t j = 0; j < ulp.size(); ++j)
            {
                if (isnan(ulp[j]))
                {
                    continue;
                }
                if (clip > 0 && abs(ulp[j]) > clip)
                {
                    continue;
                }
                CoarseReal x = x_scale(coarse_abscissas_[j]);
                PreciseReal y = y_scale(ulp[j]);
                fs << "<circle cx='" << x << "' cy='" << y << "' r='1' fill='" << color << "'/>";
            }
        }

        if (ulp_envelope)
        {
            // chartreuse?
            std::string close_path = "' stroke='chartreuse' stroke-width='1' fill='none'></path>\n";
            size_t jstart = 0;
            if (clip > 0)
            {
                while (cond_[jstart] > clip)
                {
                    ++jstart;
                    if (jstart >= cond_.size())
                    {
                        goto done;
                    }
                }
            }
            size_t jmin = jstart;
new_top_path:
            if (jmin >= cond_.size()) {
                goto done;
            }
            fs << "<path d='M" << x_scale(coarse_abscissas_[jmin]) << " " << y_scale(cond_[jmin]);

            for (size_t j = jmin + 1; j < coarse_abscissas_.size(); ++j)
            {
                bool bad = isnan(cond_[j]) || (clip > 0 && cond_[j] > clip);
                if (bad)
                {
                    ++j;
                    while ( (j < coarse_abscissas_.size() - 2) && bad)
                    {
                        bad = isnan(cond_[j]) || (clip > 0 && cond_[j] > clip);
                        ++j;
                    }
                    jmin = j;
                    fs << close_path;
                    goto new_top_path;
                }

                CoarseReal t = x_scale(coarse_abscissas_[j]);
                PreciseReal y = y_scale(cond_[j]);
                fs << " L" << t << " " << y;
            }
            fs << close_path;
            jmin = jstart;
new_bottom_path:
            fs << "<path d='M" << x_scale(coarse_abscissas_[jmin]) << " " << y_scale(-cond_[jmin]);

            for (size_t j = jmin + 1; j < coarse_abscissas_.size(); ++j)
            {
                bool bad = isnan(cond_[j]) || (clip > 0 && cond_[j] > clip);
                if (bad)
                {
                    ++j;
                    while ( (j < coarse_abscissas_.size() - 2) && bad)
                    {
                        bad = isnan(cond_[j]) || (clip > 0 && cond_[j] > clip);
                        ++j;
                    }
                    jmin = j;
                    fs << close_path;
                    goto new_bottom_path;
                }
                CoarseReal t = x_scale(coarse_abscissas_[j]);
                PreciseReal y = y_scale(-cond_[j]);
                fs << " L" << t << " " << y;
            }
            fs << close_path;
        }

done:
        fs << "</g>\n"
           << "</svg>\n";
        fs.close();
    }

private:
    std::vector<PreciseReal> precise_abscissas_;
    std::vector<CoarseReal> coarse_abscissas_;
    std::vector<PreciseReal> precise_ordinates_;
    std::vector<PreciseReal> cond_;
    std::list<std::vector<CoarseReal>> ulp_list_;
    std::vector<std::string> colors_;
    CoarseReal a_;
    CoarseReal b_;
};

} // namespace quicksvg
#endif
