use std::{process::exit, time::Instant};

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
        let num_vertices = csc.num_of_vertices;
        let num_edges = csc.num_of_edges;
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

    pub fn trim(&mut self) {
        let start = Instant::now();
        let mut has_in = vec![false; self.num_vertices];
        let mut has_out = vec![false; self.num_vertices];

        for v in 0..self.num_vertices {
            for j in self.csc.com[v]..self.csc.com[v + 1] {
                if !self.removed[v] && self.csc.unc[j] != v {
                    has_in[v] = true;
                    has_out[self.csc.unc[j]] = true;
                }
            }
        }

        for v in 0..self.num_vertices {
            if !has_in[v] || !has_out[v] {
                self.removed[v] = true;
                self.num_trimmed += 1;
            }
        }
        log::trace!("Trimmed {} vertices", self.num_trimmed);
        log::trace!("Trimming took: {:?}", start.elapsed());
    }
}
