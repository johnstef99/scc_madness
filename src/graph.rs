use std::process::exit;

use crate::csx::CSX;

#[derive(Debug)]
pub struct Graph {
    pub num_vertices: usize,

    pub num_edges: usize,

    pub csc: CSX,

    pub removed: Vec<bool>,

    pub scc_id: Vec<usize>,

    pub num_trimmed: usize,
}

impl Graph {
    pub fn new(csc: CSX) -> Self {
        let num_vertices = csc.num_of_vertices();
        let num_edges = csc.num_of_edges();
        let removed = vec![false; num_vertices];
        let scc_id = vec![0; num_vertices];
        let num_trimmed = 0;

        Self {
            num_vertices,
            num_edges,
            csc,
            removed,
            scc_id,
            num_trimmed,
        }
    }

    pub fn from_file(filename: &str) -> Self {
        let csc = match CSX::from_file(filename) {
            Ok(csc) => csc,
            Err(e) => {
                eprintln!("Error: {}", e);
                exit(1);
            }
        };
        Self::new(csc)
    }
}
