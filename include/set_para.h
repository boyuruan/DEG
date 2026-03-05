#include "parameters.h"
#include <cmath>
#include <iostream>

namespace
{
    inline unsigned get_num_vectors(stkq::Parameters const &parameters)
    {
        try
        {
            return parameters.get<unsigned>("num_vectors");
        }
        catch (...)
        {
            return 2;
        }
    }
}

inline void HNSW_PARA(std::string dataset, stkq::Parameters &parameters)
{
    unsigned max_m, max_m0, ef_construction;
    if (dataset == "openimage")
    {
        max_m = 40, max_m0 = 40, ef_construction = 200;
    }
    else if (dataset == "sg-ins")
    {
        max_m = 40, max_m0 = 40, ef_construction = 200;
    }
    else if (dataset == "howto100m")
    {
        max_m = 40, max_m0 = 40, ef_construction = 200;
    }
    else if (dataset == "cc3m")
    {
        max_m = 40, max_m0 = 40, ef_construction = 200;
    }
    else if (dataset == "Twitter10M")
    {
        max_m = 40, max_m0 = 40, ef_construction = 200;
    }
    else
    {
        std::cout << "dataset error!\n";
        exit(-1);
    }
    parameters.set<int>("mult", -1);
    parameters.set<unsigned>("max_m", max_m);
    parameters.set<unsigned>("max_m0", max_m0);
    parameters.set<unsigned>("ef_construction", ef_construction);
}

inline void BS4_PARA(std::string dataset, stkq::Parameters &parameters)
{
    unsigned max_m, max_m0, ef_construction;
    if (dataset == "openimage")
    {
        max_m = 40, max_m0 = 40, ef_construction = 200;
    }
    else if (dataset == "sg-ins")
    {
        max_m = 10, max_m0 = 10, ef_construction = 50;
    }
    else if (dataset == "howto100m")
    {
        max_m = 10, max_m0 = 10, ef_construction = 50;
    }
    else if (dataset == "cc3m")
    {
        max_m = 10, max_m0 = 10, ef_construction = 50;
    }
    else if (dataset == "Twitter10M")
    {
        max_m = 10, max_m0 = 10, ef_construction = 50;
    }
    else
    {
        std::cout << "dataset error!\n";
        exit(-1);
    }
    parameters.set<int>("mult", -1);
    parameters.set<unsigned>("max_m", max_m);
    parameters.set<unsigned>("max_m0", max_m0);
    parameters.set<unsigned>("ef_construction", ef_construction);
}

inline void DEG_PARA(std::string dataset, stkq::Parameters &parameters)
{
    unsigned max_m, ef_construction;
    if (dataset == "openimage")
    {
        max_m = 40, ef_construction = 200;
    }
    else if (dataset == "sg-ins")
    {
        max_m = 40, ef_construction = 200;
    }
    else if (dataset == "howto100m")
    {
        max_m = 40, ef_construction = 200;
    }
    else if (dataset == "cc3m")
    {
        max_m = 40, ef_construction = 200;
    }
    else if (dataset == "Twitter10M")
    {
        max_m = 40, ef_construction = 200;
    }
    else
    {
        std::cout << "dataset error!\n";
        exit(-1);
    }
    parameters.set<unsigned>("max_m", max_m);
    parameters.set<unsigned>("ef_construction", ef_construction);
    parameters.set<int>("mult", -1);
}

inline void set_data_path(std::string dataset, stkq::Parameters &parameters)
{
    unsigned num_vectors = get_num_vectors(parameters);
    std::string dataset_root = parameters.get<std::string>("dataset_root");

    std::string dataset_dir;
    if (dataset == "openimage")
        dataset_dir = "OpenImage";
    else if (dataset == "sg-ins")
        dataset_dir = "SG-ins";
    else if (dataset == "howto100m")
        dataset_dir = "howto100m";
    else if (dataset == "cc3m")
        dataset_dir = "CC3M";
    else if (dataset == "Twitter10M")
        dataset_dir = "Twitter10M";
    else
    {
        std::cout << "dataset input error!\n";
        exit(-1);
    }

    float alpha = parameters.get<float>("alpha");
    int range = 0;
    std::cout << alpha << std::endl;
    const float epsilon = 1e-6;
    if (std::fabs(alpha - 0) < epsilon)
        range = 0;
    else if (std::fabs(alpha - 1) < epsilon)
        range = 6;
    else if (std::fabs(alpha - 0.1f) < epsilon)
        range = 1;
    else if (std::fabs(alpha - 0.3f) < epsilon)
        range = 2;
    else if (std::fabs(alpha - 0.5f) < epsilon)
        range = 3;
    else if (std::fabs(alpha - 0.7f) < epsilon)
        range = 4;
    else if (std::fabs(alpha - 0.9f) < epsilon)
        range = 5;
    else
    {
        std::cout << "alpha input error!\n";
        exit(-1);
    }
    std::cout << "Range: " << range << std::endl;

    std::string dir_prefix = dataset_root + dataset_dir + "/";
    std::string ground_path = dir_prefix + "range_" + std::to_string(range) + "_top10_results.ivecs";
    parameters.set<std::string>("ground_path", ground_path);

    if (num_vectors > 2)
    {
        for (unsigned i = 0; i < num_vectors; ++i)
        {
            parameters.set<std::string>("base_vec_path_" + std::to_string(i),
                                        dir_prefix + "base_vec_" + std::to_string(i) + ".fvecs");
            parameters.set<std::string>("query_vec_path_" + std::to_string(i),
                                        dir_prefix + "query_vec_" + std::to_string(i) + ".fvecs");
        }
        parameters.set<std::string>("query_weights_path",
                                    dir_prefix + "query_weights.fvecs");
        parameters.set<std::string>("base_emb_path", parameters.get<std::string>("base_vec_path_0"));
        parameters.set<std::string>("base_loc_path", parameters.get<std::string>("base_vec_path_1"));
        parameters.set<std::string>("query_emb_path", parameters.get<std::string>("query_vec_path_0"));
        parameters.set<std::string>("query_loc_path", parameters.get<std::string>("query_vec_path_1"));
        parameters.set<std::string>("query_alpha_path", parameters.get<std::string>("query_weights_path"));
        return;
    }

    std::string base_emb_path(dataset_root);
    std::string base_loc_path(dataset_root);
    std::string query_emb_path(dataset_root);
    std::string query_loc_path(dataset_root);
    std::string query_alpha_path(dataset_root);

    if (dataset == "openimage")
    {
        base_emb_path.append(R"(OpenImage/base_img_emb.fvecs)");
        base_loc_path.append(R"(OpenImage/base_text_emb.fvecs)");
        query_emb_path.append(R"(OpenImage/query_img_emb.fvecs)");
        query_loc_path.append(R"(OpenImage/query_text_emb.fvecs)");
        query_alpha_path.append(R"(OpenImage/range_)" + std::to_string(range) + "_query_alpha.fvecs");
    }
    else if (dataset == "sg-ins")
    {
        base_emb_path.append(R"(SG-ins/base_emb.fvecs)");
        base_loc_path.append(R"(SG-ins/base_loc.fvecs)");
        query_emb_path.append(R"(SG-ins/query_emb.fvecs)");
        query_loc_path.append(R"(SG-ins/new_query_loc.fvecs)");
        query_alpha_path.append(R"(SG-ins/range_)" + std::to_string(range) + "_query_alpha.fvecs");
    }
    else if (dataset == "howto100m")
    {
        base_emb_path.append(R"(howto100m/base_img_emb.fvecs)");
        base_loc_path.append(R"(howto100m/base_text_emb.fvecs)");
        query_emb_path.append(R"(howto100m/query_img_emb.fvecs)");
        query_loc_path.append(R"(howto100m/query_text_emb.fvecs)");
        query_alpha_path.append(R"(howto100m/range_)" + std::to_string(range) + "_query_alpha.fvecs");
    }
    else if (dataset == "cc3m")
    {
        base_emb_path.append(R"(CC3M/base_img_emb.fvecs)");
        base_loc_path.append(R"(CC3M/base_text_emb.fvecs)");
        query_emb_path.append(R"(CC3M/query_img_emb.fvecs)");
        query_loc_path.append(R"(CC3M/query_text_emb.fvecs)");
        query_alpha_path.append(R"(CC3M/range_)" + std::to_string(range) + "_query_alpha.fvecs");
    }
    else if (dataset == "Twitter10M")
    {
        base_emb_path.append(R"(Twitter10M/base_emb.fvecs)");
        base_loc_path.append(R"(Twitter10M/sample_base_loc.fvecs)");
        query_emb_path.append(R"(Twitter10M/query_emb.fvecs)");
        query_loc_path.append(R"(Twitter10M/query_loc.fvecs)");
        query_alpha_path.append(R"(Twitter10M/range_)" + std::to_string(range) + "_query_alpha.fvecs");
    }

    parameters.set<std::string>("base_emb_path", base_emb_path);
    parameters.set<std::string>("base_loc_path", base_loc_path);
    parameters.set<std::string>("query_emb_path", query_emb_path);
    parameters.set<std::string>("query_loc_path", query_loc_path);
    parameters.set<std::string>("query_alpha_path", query_alpha_path);

    parameters.set<std::string>("base_vec_path_0", base_emb_path);
    parameters.set<std::string>("base_vec_path_1", base_loc_path);
    parameters.set<std::string>("query_vec_path_0", query_emb_path);
    parameters.set<std::string>("query_vec_path_1", query_loc_path);
    parameters.set<std::string>("query_weights_path", query_alpha_path);
}

inline void set_para(std::string alg, std::string dataset, stkq::Parameters &parameters)
{
    set_data_path(dataset, parameters);
    if (parameters.get<std::string>("exc_type") != "build")
    {
        return;
    }

    if (alg == "hnsw")
    {
        HNSW_PARA(dataset, parameters);
    }
    else if (alg == "deg")
    {
        DEG_PARA(dataset, parameters);
    }
    else if (alg == "baseline1")
    {
        HNSW_PARA(dataset, parameters);
    }
    else if (alg == "baseline2")
    {
        HNSW_PARA(dataset, parameters);
    }
    else if (alg == "baseline3")
    {
        HNSW_PARA(dataset, parameters);
    }
    else if (alg == "baseline4")
    {
        BS4_PARA(dataset, parameters);
    }
    else
    {
        std::cout << "algorithm input error!\n";
        exit(-1);
    }
}