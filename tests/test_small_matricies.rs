use scc_madness::graph::Graph;

fn test_matrix(filename: &str, num_trimmed: usize, num_of_scc: usize) {
    let mut graph = Graph::from_file(filename);
    unsafe { graph.trim_par_unsafe().unwrap() };
    assert_eq!(graph.num_trimmed, num_trimmed);
    unsafe { graph.color_scc_par_unsafe() };
    assert_eq!(graph.num_of_scc(), num_of_scc);
    std::mem::drop(graph);

    let mut graph = Graph::from_file(filename);
    graph.trim_par().unwrap();
    assert_eq!(graph.num_trimmed, num_trimmed);
    graph.color_scc_par();
    assert_eq!(graph.num_of_scc(), num_of_scc);
    std::mem::drop(graph);

    let mut graph = Graph::from_file(filename);
    graph.trim();
    assert_eq!(graph.num_trimmed, num_trimmed);
    graph.color_scc();
    assert_eq!(graph.num_of_scc(), num_of_scc);
    std::mem::drop(graph);
}

#[test]
fn mine_matrix() {
    test_matrix("assets/mine.mtx", 1, 3);
}

#[test]
fn celegansneural_matrix() {
    test_matrix("assets/celegansneural.mtx", 30, 57);
}

#[test]
fn foldoc_matrix() {
    test_matrix("assets/foldoc.mtx", 47, 71);
}
