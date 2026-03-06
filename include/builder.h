
#ifndef STKQ_BUILDER_H
#define STKQ_BUILDER_H

#include "index.h"
#include <vector>

namespace stkq
{
    class IndexBuilder
    {
    public:
        explicit IndexBuilder(const unsigned num_threads, const float max_emb_dist, const float max_spatial_dist, bool dual_index = false)
            : final_index_(nullptr), final_index_1(nullptr), final_index_2(nullptr)
        {
            if (!dual_index)
            {
                final_index_ = new Index(max_emb_dist, max_spatial_dist);
                omp_set_num_threads(num_threads);
            }
            else
            {
                final_index_ = new Index(max_emb_dist, max_spatial_dist);
                final_index_1 = new Index(max_emb_dist, max_spatial_dist);
                final_index_2 = new Index(max_emb_dist, max_spatial_dist);
                omp_set_num_threads(num_threads);
            }
        }

        explicit IndexBuilder(const unsigned num_threads, unsigned num_vectors,
                              std::vector<float> const &max_distances, bool dual_index = false)
            : final_index_(nullptr), final_index_1(nullptr), final_index_2(nullptr)
        {
            final_index_ = new Index(num_vectors, max_distances);
            omp_set_num_threads(num_threads);
            if (dual_index && num_vectors > 2)
            {
                for (unsigned i = 0; i < num_vectors; ++i)
                    nway_indices_.push_back(new Index(num_vectors, max_distances));
                final_index_1 = nway_indices_[0];
                final_index_2 = nway_indices_.size() > 1 ? nway_indices_[1] : nullptr;
            }
            else if (dual_index)
            {
                final_index_1 = new Index(num_vectors, max_distances);
                final_index_2 = new Index(num_vectors, max_distances);
            }
        }

        virtual ~IndexBuilder()
        {
            delete final_index_;
            if (nway_indices_.empty())
            {
                delete final_index_1;
                delete final_index_2;
            }
            else
            {
                for (auto *idx : nway_indices_)
                    delete idx;
            }
        }

        IndexBuilder *load(char *data_emb_file, char *data_loc_file, char *query_emb_file, char *query_loc_file, char *query_alpha_file, char *ground_file, Parameters &parameters, bool dual = false);

        IndexBuilder *init(TYPE type, bool debug = false);

        IndexBuilder *save_graph(TYPE type, char *graph_file);

        IndexBuilder *load_graph(TYPE type, char *graph_file);

        IndexBuilder *load_graph(TYPE type, char *graph_file_1, char *graph_file_2);

        IndexBuilder *load_graph(TYPE type, std::vector<std::string> const &graph_files);

        IndexBuilder *refine(TYPE type, bool debug);

        IndexBuilder *search(TYPE entry_type, TYPE route_type, TYPE L_type, Parameters para_);

        void print_graph();

        void degree_info(std::unordered_map<unsigned, unsigned> &in_degree, std::unordered_map<unsigned, unsigned> &out_degree, TYPE type);

        void conn_info(TYPE type);

        void graph_quality(TYPE type);

        void DFS(boost::dynamic_bitset<> &flag, unsigned root, unsigned &cnt, TYPE type);

        void findRoot(boost::dynamic_bitset<> &flag, std::vector<unsigned> &root);

        void set_begin_time()
        {
            s = std::chrono::high_resolution_clock::now();
        }

        void set_end_time()
        {
            e = std::chrono::high_resolution_clock::now();
        }

        IndexBuilder *draw();

        std::chrono::duration<double> GetBuildTime() { return e - s; }

        void peak_memory_footprint();

        Index *getIndex() { return final_index_; }

    private:
        Index *final_index_;
        Index *final_index_1;
        Index *final_index_2;
        std::vector<Index *> nway_indices_;

        std::chrono::high_resolution_clock::time_point s;
        std::chrono::high_resolution_clock::time_point e;
    };
}

#endif