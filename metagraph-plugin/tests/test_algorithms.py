import metagraph as mg
import numpy as np
import pytest

def test_bfs(networkx_weighted_directed_8_12, kg_from_nx_di_8_12):
    bfs1_nx = mg.algos.traversal.bfs_iter(networkx_weighted_directed_8_12, 0)
    bfs2_nx = mg.algos.traversal.bfs_iter(networkx_weighted_directed_8_12, 2)
    bfs1_kg = mg.algos.traversal.bfs_iter(kg_from_nx_di_8_12, 0)
    bfs2_kg = mg.algos.traversal.bfs_iter(kg_from_nx_di_8_12, 2)
    assert bfs1_kg.tolist() == bfs1_nx.tolist()
    assert bfs2_kg.tolist() == bfs2_nx.tolist()
    assert bfs1_kg.tolist() == [0, 1, 3, 4, 7]
    assert bfs2_kg.tolist() == [2, 4, 5, 6, 7]


@pytest.mark.xfail(reason="until BFS implementation is fixed.")
def test_bfs_iter(networkx_weighted_directed_bfs, kg_from_nx_di_bfs):
    bfs1_nx = mg.algos.traversal.bfs_iter(networkx_weighted_directed_bfs, 0)
    bfs1_kg = mg.algos.traversal.bfs_iter(kg_from_nx_di_bfs, 0)
    assert bfs1_kg.tolist() == bfs1_nx.tolist()
    assert bfs1_kg.tolist() == [0, 3, 4, 5, 6, 2, 7]


def test_bfs_kg(katanagraph_cleaned_8_12_di, nx_from_kg_di_8_12):
    '''
    test for katana graph which is directly loaded rather than translated from nettworkx, also test two consecutive runs with the same source code
    '''
    src_node = 2
    bfs1_kg = mg.algos.traversal.bfs_iter(katanagraph_cleaned_8_12_di, src_node)
    bfs2_kg = mg.algos.traversal.bfs_iter(katanagraph_cleaned_8_12_di, src_node)
    bfs_nx = mg.algos.traversal.bfs_iter(nx_from_kg_di_8_12, src_node)
    assert bfs1_kg.tolist() == bfs2_kg.tolist()
    assert len(bfs1_kg.tolist()) > 0
    assert bfs1_kg.tolist() == bfs_nx.tolist()



def test_sssp_bellman_ford(networkx_weighted_directed_8_12, kg_from_nx_di_8_12):
    src_node = 0
    sssp_nx = mg.algos.traversal.bellman_ford(networkx_weighted_directed_8_12, src_node)  # source node is 0
    parents_nx = sssp_nx[0]
    distances_nx = sssp_nx[1]
    assert isinstance(parents_nx, dict)
    assert isinstance(distances_nx, dict)
    assert parents_nx == {0: 0, 1: 0, 3: 0, 4: 3, 7: 4}
    assert distances_nx == {0: 0, 1: 4, 3: 2, 4: 3, 7: 7}
    parents_kg, distances_kg = mg.algos.traversal.bellman_ford(kg_from_nx_di_8_12, src_node)
    assert parents_nx == parents_kg
    assert distances_nx == distances_kg


@pytest.mark.runslow
def test_sssp_bellman_ford_kg(katanagraph_cleaned_8_12_di, nx_from_kg_di_8_12):
    '''
    test for katana graph which is directly loaded rather than translated from nettworkx, also test two consecutive runs with the same source code
    '''
    src_node = 0
    sssp1_kg = mg.algos.traversal.bellman_ford(katanagraph_cleaned_8_12_di, src_node)
    sssp2_kg = mg.algos.traversal.bellman_ford(katanagraph_cleaned_8_12_di, src_node)
    sssp_nx = mg.algos.traversal.bellman_ford(nx_from_kg_di_8_12, src_node)
    assert sssp1_kg[0] == sssp2_kg[0]
    assert sssp1_kg[1] == sssp2_kg[1]
    assert sssp1_kg[0] == sssp_nx[0]
    assert sssp1_kg[1] == sssp_nx[1]


def test_sssp_dijkstra(networkx_weighted_directed_8_12, kg_from_nx_di_8_12):
    src_node = 1
    sssp_nx = mg.algos.traversal.dijkstra(networkx_weighted_directed_8_12, src_node)  # source node is 1
    parents_nx = sssp_nx[0]
    distances_nx = sssp_nx[1]
    assert isinstance(parents_nx, dict)
    assert isinstance(distances_nx, dict)
    assert parents_nx == {1: 1, 3: 1, 4: 3, 7: 4}
    assert distances_nx == {1: 0, 3: 3, 4: 4, 7: 8}
    parents_kg, distances_kg = mg.algos.traversal.dijkstra(kg_from_nx_di_8_12, src_node)
    assert parents_nx == parents_kg
    assert distances_nx == distances_kg


@pytest.mark.runslow
def test_sssp_dijkstra_kg(katanagraph_cleaned_8_12_di, nx_from_kg_di_8_12):
    '''
    test for katana graph which is directly loaded rather than translated from nettworkx, also test two consecutive runs with the same source code
    '''
    src_node = 1
    sssp1_kg = mg.algos.traversal.dijkstra(katanagraph_cleaned_8_12_di, src_node)
    sssp2_kg = mg.algos.traversal.dijkstra(katanagraph_cleaned_8_12_di, src_node)
    sssp_nx = mg.algos.traversal.dijkstra(nx_from_kg_di_8_12, src_node)
    assert sssp1_kg[0] == sssp2_kg[0]
    assert sssp1_kg[1] == sssp2_kg[1]
    assert sssp1_kg[0] == sssp_nx[0]
    assert sssp1_kg[1] == sssp_nx[1]


def test_connected_components(networkx_weighted_undirected_8_12, kg_from_nx_ud_8_12):
    cc_nx = mg.algos.clustering.connected_components(networkx_weighted_undirected_8_12)
    cc_kg = mg.algos.clustering.connected_components(kg_from_nx_ud_8_12)
    assert isinstance(cc_kg, dict)
    assert isinstance(cc_kg, dict)
    assert cc_kg == cc_nx


@pytest.mark.runslow
def test_connected_components_kg(katanagraph_cleaned_8_12_ud, nx_from_kg_ud_8_12):
    '''
    test for katana graph which is directly loaded rather than translated from nettworkx, also test two consecutive runs with the same source code
    '''
    cc_kg1 = mg.algos.clustering.connected_components(katanagraph_cleaned_8_12_ud)
    cc_kg2 = mg.algos.clustering.connected_components(katanagraph_cleaned_8_12_ud)
    cc_nx = mg.algos.clustering.connected_components(nx_from_kg_ud_8_12)
    assert cc_kg1 == cc_kg2
    assert cc_kg1 == cc_nx


def test_pagerank(networkx_weighted_directed_8_12, kg_from_nx_di_8_12):
    pr_nx = mg.algos.centrality.pagerank(networkx_weighted_directed_8_12)
    pr_kg = mg.algos.centrality.pagerank(kg_from_nx_di_8_12)
    assert isinstance(pr_nx, dict)
    assert isinstance(pr_kg, dict)
    assert pr_nx == pr_kg


@pytest.mark.runslow
def test_pagerank_kg(katanagraph_cleaned_8_12_di, nx_from_kg_di_8_12):
    '''
    test for katana graph which is directly loaded rather than translated from nettworkx, also test two consecutive runs with the same source code
    '''
    pr_kg1 = mg.algos.centrality.pagerank(katanagraph_cleaned_8_12_di)
    pr_kg2 = mg.algos.centrality.pagerank(katanagraph_cleaned_8_12_di)
    pr_nx = mg.algos.centrality.pagerank(nx_from_kg_di_8_12)
    assert pr_kg1 == pr_kg2
    assert pr_kg1 == pr_nx


def test_betweenness_centrality(networkx_weighted_directed_8_12, kg_from_nx_di_8_12):
    bc_nx = mg.algos.centrality.betweenness(networkx_weighted_directed_8_12)
    bc_kg = mg.algos.centrality.betweenness(kg_from_nx_di_8_12)
    assert isinstance(bc_nx, dict)
    assert isinstance(bc_kg, dict)
    assert bc_nx == bc_kg


@pytest.mark.runslow
def test_betweenness_centrality_kg(katanagraph_cleaned_8_12_di, nx_from_kg_di_8_12):
    '''
    test for katana graph which is directly loaded rather than translated from nettworkx, also test two consecutive runs with the same source code
    Notice for large graphs, mg.algos.centrality.betweenness is extremely slow (not because of our translator)
    '''
    bc_kg1 = mg.algos.centrality.betweenness(katanagraph_cleaned_8_12_di)
    bc_kg2 = mg.algos.centrality.betweenness(katanagraph_cleaned_8_12_di)
    assert bc_kg1 == bc_kg2
    bc_nx = mg.algos.centrality.betweenness(nx_from_kg_di_8_12)
    assert bc_kg1 == bc_nx


def test_triangle_counting(networkx_weighted_undirected_8_12, kg_from_nx_ud_8_12):
    tc_nx = mg.algos.clustering.triangle_count(networkx_weighted_undirected_8_12)
    tc_kg = mg.algos.clustering.triangle_count(kg_from_nx_ud_8_12)
    assert isinstance(tc_nx, int)
    assert isinstance(tc_kg, int)
    assert tc_nx == tc_kg


@pytest.mark.runslow
def test_triangle_counting_kg(katanagraph_cleaned_8_12_ud, nx_from_kg_ud_8_12):
    '''
    test for katana graph which is directly loaded rather than translated from nettworkx, also test two consecutive runs with the same source code
    '''
    tc_kg1 = mg.algos.clustering.triangle_count(katanagraph_cleaned_8_12_ud)
    tc_kg2 = mg.algos.clustering.triangle_count(katanagraph_cleaned_8_12_ud)
    tc_nx = mg.algos.clustering.triangle_count(nx_from_kg_ud_8_12)
    assert tc_kg1 == tc_kg2
    assert tc_kg1 == tc_nx


def test_louvain_community_detection(networkx_weighted_undirected_8_12, kg_from_nx_ud_8_12):
    lc_nx = mg.algos.clustering.louvain_community(networkx_weighted_undirected_8_12)
    lc_kg = mg.algos.clustering.louvain_community(kg_from_nx_ud_8_12)
    assert isinstance(lc_nx[0], dict)
    assert isinstance(lc_kg[0], dict)
    assert isinstance(lc_nx[1], float)
    assert isinstance(lc_kg[1], float)
    assert lc_nx[0] == lc_kg[0]
    assert lc_nx[1] == lc_kg[1]


@pytest.mark.runslow
def test_louvain_community_detection_kg(katanagraph_cleaned_8_12_ud, nx_from_kg_ud_8_12):
    '''
    test for katana graph which is directly loaded rather than translated from nettworkx.
    We cannot expect two consecutive runs with the same source code give the same results.
    The reason is two runs use different random seeds.
    Besides, we cannot set the random seed cause the metagraph wrapper hide that option in network's community_louvain.best_partition function
    '''
    lc_kg1 = mg.algos.clustering.louvain_community(katanagraph_cleaned_8_12_ud)
    lc_kg2 = mg.algos.clustering.louvain_community(katanagraph_cleaned_8_12_ud)
    lc_nx = mg.algos.clustering.louvain_community(nx_from_kg_ud_8_12)
    # assert lc_kg1[0] == lc_kg2[0] # failed cause two runs get different results (different random seeds from network's community_louvain.best_partition function).
    assert abs(lc_kg1[1] - lc_kg2[1]) < 0.1
    assert lc_kg1[0] == lc_nx[0]
    assert lc_kg1[1] == lc_nx[1]


def test_translation_subgraph_extraction(networkx_weighted_directed_8_12, kg_from_nx_di_8_12):
    se_nx = mg.algos.subgraph.extract_subgraph(networkx_weighted_directed_8_12, {0, 2, 3})
    se_kg = mg.algos.subgraph.extract_subgraph(kg_from_nx_di_8_12, {0, 2, 3})
    assert isinstance(se_nx, mg.wrappers.Graph.NetworkXGraph)
    assert isinstance(se_kg, mg.wrappers.Graph.NetworkXGraph)
    assert list(se_nx.value.edges(data=True)) == list(se_kg.value.edges(data=True))


@pytest.mark.runslow
def test_translation_subgraph_extraction_kg(katanagraph_cleaned_8_12_di, nx_from_kg_di_8_12):
    '''
    test for katana graph which is directly loaded rather than translated from nettworkx, also test two consecutive runs with the same source code
    '''
    ids = {0, 4, 5}
    se_kg1 = mg.algos.subgraph.extract_subgraph(katanagraph_cleaned_8_12_di, ids)
    se_kg2 = mg.algos.subgraph.extract_subgraph(katanagraph_cleaned_8_12_di, ids)
    se_nx = mg.algos.subgraph.extract_subgraph(nx_from_kg_di_8_12, ids)
    assert list(se_kg1.value.edges(data=True)) == list(se_kg2.value.edges(data=True))
    assert list(se_kg1.value.edges(data=True)) == list(se_nx.value.edges(data=True))


def test_labal_propagation(networkx_weighted_undirected_8_12, kg_from_nx_ud_8_12):
    cd_nx = mg.algos.clustering.label_propagation_community(networkx_weighted_undirected_8_12)
    cd_kg = mg.algos.clustering.label_propagation_community(kg_from_nx_ud_8_12)
    assert isinstance(cd_nx, dict)
    assert isinstance(cd_kg, dict)
    assert cd_nx == cd_kg


@pytest.mark.runslow
def test_labal_propagation_kg(katanagraph_cleaned_8_12_ud, nx_from_kg_ud_8_12):
    '''
    test for katana graph which is directly loaded rather than translated from nettworkx, also test two consecutive runs with the same source code
    '''
    cd_kg1 = mg.algos.clustering.label_propagation_community(katanagraph_cleaned_8_12_ud)
    cd_kg2 = mg.algos.clustering.label_propagation_community(katanagraph_cleaned_8_12_ud)
    cd_nx = mg.algos.clustering.label_propagation_community(nx_from_kg_ud_8_12)
    assert cd_kg1 == cd_kg2
    assert cd_kg1 == cd_nx


def test_jaccard_similarity(networkx_weighted_undirected_8_12, kg_from_nx_ud_8_12):
    compare_node = 0
    prop_name = "jaccard_prop_with_" + str(compare_node)
    jcd_nx = mg.algos.traversal.jaccard(networkx_weighted_undirected_8_12, compare_node)
    jcd_kg = mg.algos.traversal.jaccard(kg_from_nx_ud_8_12, compare_node)
    assert isinstance(jcd_nx, np.ndarray)
    assert isinstance(jcd_kg, np.ndarray)
    assert jcd_nx.tolist() == jcd_kg.tolist()
    assert jcd_kg[compare_node] == 1


@pytest.mark.runslow
def test_jaccard_similarity_kg(katanagraph_cleaned_8_12_ud, nx_from_kg_ud_8_12):
    '''
    test for katana graph which is directly loaded rather than translated from nettworkx, also test two consecutive runs with the same source code
    '''
    compare_node = 3
    prop_name = "jaccard_prop_with_" + str(compare_node)
    jcd_kg1 = mg.algos.traversal.jaccard(katanagraph_cleaned_8_12_ud, compare_node)
    jcd_kg2 = mg.algos.traversal.jaccard(katanagraph_cleaned_8_12_ud, compare_node)
    assert jcd_kg1.tolist() == jcd_kg2.tolist()
    assert jcd_kg1[compare_node] == 1
    assert jcd_kg2[compare_node] == 1
    jcd_nx = mg.algos.traversal.jaccard(nx_from_kg_ud_8_12, compare_node)
    assert jcd_nx[compare_node] == 1
    assert jcd_kg1.tolist() == jcd_nx.tolist()


def test_local_clustering_coefficient(networkx_weighted_undirected_8_12, kg_from_nx_ud_8_12):
    prop_name = "output_prop"
    lcc_nx = mg.algos.clustering.local_clustering_coefficient(networkx_weighted_undirected_8_12, prop_name)
    lcc_kg = mg.algos.clustering.local_clustering_coefficient(kg_from_nx_ud_8_12, prop_name)
    assert isinstance(lcc_nx, np.ndarray)
    assert isinstance(lcc_kg, np.ndarray)
    assert lcc_kg.tolist() == lcc_nx.tolist()
    assert lcc_kg[-1] == 0
    assert not np.any(np.isnan(lcc_kg))


@pytest.mark.runslow
def test_local_clustering_coefficient_kg(katanagraph_cleaned_8_12_ud, nx_from_kg_ud_8_12):
    '''
    test for katana graph which is directly loaded rather than translated from nettworkx, also test two consecutive runs with the same source code
    '''
    prop_name = "output_prop"
    lcc_kg1 = mg.algos.clustering.local_clustering_coefficient(katanagraph_cleaned_8_12_ud, prop_name)
    lcc_kg2 = mg.algos.clustering.local_clustering_coefficient(katanagraph_cleaned_8_12_ud, prop_name)
    assert lcc_kg1.tolist() == lcc_kg2.tolist()
    assert lcc_kg1[-1] == 0
    assert not np.any(np.isnan(lcc_kg1))
    lcc_nx = mg.algos.clustering.local_clustering_coefficient(nx_from_kg_ud_8_12, prop_name)
    assert lcc_kg1.tolist() == lcc_nx.tolist()
