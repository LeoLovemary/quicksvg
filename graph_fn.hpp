#ifndef QUICKSVG_GRAPH_FN_HPP
#define QUICKSVG_GRAPH_FN_HPP
#include "detail/generic_svg_functionality.hpp"
#include <iomanip>
#include <cassert>
#include <vector>
#include <utility>
#include <fstream>

namespace quicksvg {

class graph_fn {
public:
    graph_fn(double x_min, double x_max, std::string const & title, std::string const & filename,
             unsigned samples = 100, int width = 1100) :
             m_min_x{x_min},
             m_max_x{x_max},
             m_samples{samples},
             m_is_written{false}
    {
        m_fs.open(filename);
        assert(m_max_x > m_min_x);
        int height = floor(double(width)/1.61803);

        m_margin_top = 40;
        m_margin_left = 25;
        m_margin_bottom = 20;
        m_margin_right = 20;
        m_graph_height = height - m_margin_bottom - m_margin_top;
        m_graph_width = width - m_margin_left - m_margin_right;

        detail::write_prelude(m_fs, title, width, height, m_margin_top);
    }

    template<class F>
    void add_fn(F f, std::string const & color="steelblue")
    {
        std::cout << "Ok!" << std::endl;
        if (m_is_written)
        {
            throw std::logic_error("Cannot add data to graph after writing it.\n");
        }

        std::vector<double> v(m_samples);
        for(size_t i = 0; i < v.size(); ++i)
        {
            double step = (m_max_x - m_min_x)/(m_samples - 1.0);
            double x = m_min_x + step*i;
            v[i] = f(x);

            if (v[i] > m_max_y)
            {
              m_max_y = v[i];
            }
            if (v[i] < m_min_y)
            {
              m_min_y = v[i];
            }
        }

        m_dataset.push_back(v);
        m_connect_color.push_back(color);
    }

    void write_all()
    {

      // Maps [a,b] to [0, graph_width]
      auto x_scale = [this](double x)->double
      {
          return ((x-m_min_x)/(m_max_x - m_min_x))*static_cast<double>(m_graph_width);
      };

      auto y_scale = [this](double y)-> double
      {
        return ((m_max_y - y)/(m_max_y - m_min_y) )*static_cast<double>(m_graph_height);
      };

        // Construct SVG group to simplify the calculations slightly:
      m_fs << "<g transform='translate(" << m_margin_left << ", " << m_margin_top << ")'>\n";
           // y-axis:
      m_fs  << "<line x1='0' y1='0' x2='0' y2='" << m_graph_height
            << "' stroke='gray' stroke-width='1' />\n";
      // x-axis: If 0 is between the min a max height, place the axis at zero.
      // Otherwise, place is at the bottom of the graph.
      double x_axis_loc = m_graph_height;
      if (m_min_y <= 0 && m_max_y >= 0)
      {
          x_axis_loc = y_scale(0);
      }
      m_fs << "<line x1='0' y1='" << x_axis_loc
           << "' x2='" << m_graph_width << "' y2='" << x_axis_loc
           << "' stroke='gray' stroke-width='1' />\n";

      detail::write_gridlines(m_fs, 8, 10, x_scale, y_scale, m_min_x, m_max_x,
                              m_min_y, m_max_y, m_graph_width, m_graph_height, m_margin_left);


      double step = (m_max_x - m_min_x)/(m_samples - 1.0);
      for (size_t i = 0; i < m_dataset.size(); ++i)
      {
          auto const & v = m_dataset[i];
          std::string const & stroke = m_connect_color[i];

          m_fs << "<path d='M" << x_scale(m_min_x) << " " << y_scale(v[0]);
          for (size_t j = 1; j < v.size(); ++j)
          {
              double t = x_scale(m_min_x + j*step);
              m_fs << " L" << t << " " << y_scale(v[j]);
          }
          m_fs << "' stroke='" << stroke << "' stroke-width='3' fill='none'></path>\n";
      }

      m_fs << "</g>\n"
         << "</svg>\n";
      m_fs.close();

      m_is_written = true;
    }

    ~graph_fn()
    {
        if (!m_is_written)
        {
          std::cerr << "Did not write svg file to disk!\n";
        }
    }

private:
    double m_min_x;
    double m_max_x;
    unsigned m_samples;
    std::ofstream m_fs;
    double m_min_y;
    double m_max_y;
    bool m_is_written;
    std::vector<std::vector<double>> m_dataset;
    std::vector<std::string> m_connect_color;
    int m_margin_top;
    int m_margin_left;
    int m_margin_bottom;
    int m_margin_right;
    int m_graph_width;
    int m_graph_height;
};

} // namespace
#endif
