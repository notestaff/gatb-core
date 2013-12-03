/*****************************************************************************
 *   GATB : Genome Assembly Tool Box                                         *
 *   Authors: [R.Chikhi, G.Rizk, E.Drezen]                                   *
 *   Based on Minia, Authors: [R.Chikhi, G.Rizk], CeCILL license             *
 *   Copyright (c) INRIA, CeCILL license, 2013                               *
 *****************************************************************************/

#include <gatb/gatb_core.hpp>

using namespace std;

/********************************************************************************/

struct Stats
{
    Stats() :  nbSuccessors(0), abundance(0), nbBranching(0), abundanceBranching(0) {}
    Integer    checksumNodes;
    Integer    checksumSuccessors;
    u_int64_t  nbSuccessors;
    u_int64_t  abundance;
    u_int64_t  nbBranching;
    Integer    checksumBranching;
    u_int64_t  abundanceBranching;

    Stats& operator+= (const Stats& st)
    {
        if (this != &st)
        {
            checksumNodes      += st.checksumNodes;
            checksumSuccessors += st.checksumSuccessors;
            nbSuccessors       += st.nbSuccessors;
            abundance          += st.abundance;
            nbBranching        += st.nbBranching;
            checksumBranching  += st.checksumBranching;
            abundanceBranching += st.abundanceBranching;
        }
        return *this;
    }
};

/********************************************************************************/

int main (int argc, char* argv[])
{
    /** We create a command line parser. */
    OptionsParser parser;
    parser.add (new OptionOneParam (STR_URI_INPUT,  "graph file", true));

    try
    {
        /** We parse the user options. */
        IProperties* options = parser.parse (argc, argv);

        /** We load the graph from the provided uri. */
        Graph graph = Graph::load (options->getStr(STR_URI_INPUT));

        /** We want to gather some statistics during the iteration. */
        ThreadObject<Stats> stats;

        /*********************************************/
        /**********    ITERATE ALL NODES    **********/
        /*********************************************/
        IDispatcher::Status status = Dispatcher().iterate (graph.iterator<Node>(), [&] (const Node& node)
        {
            /** We get the Stats object for the current thread. */
            Stats& s = stats();

            /** We update the statistics. */
            s.checksumNodes  += node.kmer;
            s.abundance      += node.abundance;

            /** We retrieve the successors. */
            Graph::Vector<Node> nodeset = graph.successors<Node> (node);

            s.nbSuccessors += nodeset.size();

            /** We iterate all the successors. */
            for (size_t i=0; i<nodeset.size(); i++)  {  s.checksumSuccessors += nodeset[i].kmer;  }
        });

        /*********************************************/
        /********** ITERATE BRANCHING NODES **********/
        /*********************************************/
        Dispatcher().iterate (graph.iterator<BranchingNode>(), [&] (const BranchingNode& node)
        {
            /** We get the Stats object for the current thread. */
            Stats& s = stats();

            /** We update the statistics. */
            s.nbBranching  ++;
            s.checksumBranching  += node.kmer;
            s.abundanceBranching += node.abundance;
        });

        /** We finalize the gathered statistics. */
        stats.foreach ([&stats] (const Stats& st)  {  *stats += st;  });

        /** We dump the statistics. */
        std::cout << std::endl;
        std::cout << "nbSolids           = " << graph.iterator<Node>().size ()      << "  "  << std::endl
                  << "nbSuccessors       = " << stats->nbSuccessors                 << "  "  << std::endl
                  << "nbBranching        = " << stats->nbBranching                  << "  "  << std::endl
                  << "checkumNodes       = " << stats->checksumNodes                << "  "  << std::endl
                  << "checksumSuccessors = " << stats->checksumSuccessors           << "  "  << std::endl
                  << "checksumBranching  = " << stats->checksumBranching            << "  "  << std::endl
                  << "abundance          = " << stats->abundance                    << "  "  << std::endl
                  << "abundanceBranching = " << stats->abundanceBranching           << "  "  << std::endl
                  << "time               = " << status.time                         << "  "  << std::endl
                  << "nbCores            = " << status.nbCores                      << "  "  << std::endl
                  << std::endl;
        std::cout << std::endl;
    }
    catch (OptionFailure& e)
    {
        e.getParser().displayErrors (stdout);
        e.getParser().displayHelp   (stdout);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
