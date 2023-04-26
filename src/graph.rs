use std::{
    process::exit,
    sync::atomic::{AtomicBool, AtomicUsize, Ordering},
    time::Instant,
};

use rayon::prelude::*;

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
        let mut scc_id = vec![0; num_vertices];
        let num_trimmed = 0;

        for v in 0..num_vertices {
            scc_id[v] = v;
        }

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

    pub fn trim_par(&mut self) -> std::io::Result<()> {
        let start = Instant::now();

        let mut has_in: Vec<bool> = vec![false; self.num_vertices];

        let has_out: Vec<AtomicBool> = vec![false; self.num_vertices]
            .into_par_iter()
            .map(|x| AtomicBool::new(x))
            .collect();

        // filter out already removed vertices
        let iter = has_in
            .par_iter_mut()
            .enumerate()
            .zip(self.removed.par_iter())
            .filter_map(|(tuple, removed)| (!removed).then(|| tuple));

        // filter out vertices with no incoming or outgoing edges excluding self-loops
        iter.for_each(|(idx, hi)| {
            let range = self.csc.range_for(idx);
            let iter = self.csc.unc[range]
                .iter()
                .filter_map(|unc| (*unc != idx).then(|| &has_out[*unc]));

            for ho in iter {
                *hi = true;
                ho.store(true, Ordering::Relaxed);
            }
        });

        let trimmed = has_in
            .par_iter()
            .zip(has_out.par_iter())
            .zip(self.removed.par_iter_mut())
            .map(|((hi, ho), rm)| {
                if !*hi || !ho.load(Ordering::Relaxed) {
                    *rm = true;
                    1
                } else {
                    0
                }
            })
            .sum::<usize>();

        self.num_trimmed = trimmed;

        log::trace!("Trimmed {} vertices", self.num_trimmed);
        log::trace!("Trimming in parallel took: {:?}", start.elapsed());

        Ok(())
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

    fn is_empty(&self) -> bool {
        for v in 0..self.num_vertices {
            if !self.removed[v] {
                return false;
            }
        }
        return true;
    }

    fn is_empty_par(&self) -> bool {
        !self.removed.par_iter().any(|removed| !removed)
    }

    fn bfs(&mut self, entry: usize, colors: &[usize]) {
        self.removed[entry] = true;

        let mut fifo = Vec::new();

        for j in self.csc.com[entry]..self.csc.com[entry + 1] {
            fifo.push(self.csc.unc[j]);
        }

        let mut head = 0;

        while head != fifo.len() {
            let v = fifo[head];
            head += 1;

            if !self.removed[v] && colors[v] == entry {
                self.removed[v] = true;
                self.scc_id[v] = entry;

                for j in self.csc.com[v]..self.csc.com[v + 1] {
                    fifo.push(self.csc.unc[j]);
                }
            }
        }
    }

    fn bfs_par(
        &self,
        removed: &[AtomicBool],
        scc_id: &[AtomicUsize],
        entry: usize,
        colors: &[usize],
    ) {
        removed[entry].store(true, Ordering::Relaxed);

        let mut fifo = Vec::new();

        for j in self.csc.com[entry]..self.csc.com[entry + 1] {
            fifo.push(self.csc.unc[j]);
        }

        let mut head = 0;

        while head != fifo.len() {
            let v = fifo[head];
            head += 1;

            if !removed[v].load(Ordering::Relaxed) && colors[v] == entry {
                removed[v].store(true, Ordering::Relaxed);
                scc_id[v].store(entry, Ordering::Relaxed);

                for j in self.csc.com[v]..self.csc.com[v + 1] {
                    fifo.push(self.csc.unc[j]);
                }
            }
        }
    }

    pub fn color_scc_par(&mut self) {
        log::trace!("Start coloring scc");
        let start = Instant::now();
        let mut colors = vec![0; self.num_vertices];
        let mut old_colors;

        while !self.is_empty_par() {
            colors
                .par_iter_mut()
                .enumerate()
                .for_each(|(v, color)| match self.removed[v] {
                    true => *color = self.scc_id[v],
                    false => *color = v,
                });

            old_colors = colors.clone();

            let color_changed = AtomicBool::new(true);
            while color_changed.load(Ordering::Relaxed) {
                color_changed.store(false, Ordering::Relaxed);

                (0..self.num_vertices)
                    .into_par_iter()
                    .zip(colors.par_iter_mut())
                    .zip(self.removed.par_iter())
                    .filter_map(|((u, color), removed)| (!removed).then(|| (u, color)))
                    .for_each(|(u, color)| {
                        let range = self.csc.range_for(u);

                        self.csc.unc[range]
                            .iter()
                            .filter(|w| !self.removed[**w])
                            .for_each(|w| {
                                if *color > old_colors[*w] {
                                    *color = old_colors[*w];
                                    color_changed.store(true, Ordering::Relaxed);
                                }
                            });
                    });

                std::mem::swap(&mut colors, &mut old_colors);
            }

            let removed: Vec<AtomicBool> = self
                .removed
                .par_iter()
                .map(|r| AtomicBool::new(*r))
                .collect();
            let scc_id: Vec<AtomicUsize> = self
                .scc_id
                .par_iter()
                .map(|r| AtomicUsize::new(*r))
                .collect();

            self.removed
                .par_iter()
                .enumerate()
                .zip(colors.par_iter())
                .filter(|((i, r), color)| !**r && **color == *i)
                .for_each(|((_i, _r), color)| {
                    self.bfs_par(&removed, &scc_id, *color, &colors);
                });

            self.removed
                .par_iter_mut()
                .zip(removed.par_iter())
                .for_each(|(r, r2)| {
                    *r = r2.load(Ordering::Relaxed);
                });

            self.scc_id
                .par_iter_mut()
                .zip(scc_id.par_iter())
                .for_each(|(r, r2)| {
                    *r = r2.load(Ordering::Relaxed);
                });
        }
        log::trace!("Coloring scc in parallel took: {:?}", start.elapsed());
    }

    pub fn color_scc(&mut self) {
        log::trace!("Start coloring scc");
        let start = Instant::now();
        let mut colors = vec![0; self.num_vertices];

        while !self.is_empty() {
            for v in 0..self.num_vertices {
                match self.removed[v] {
                    true => colors[v] = self.scc_id[v],
                    false => colors[v] = v,
                }
            }

            let mut color_changed = true;
            while color_changed {
                color_changed = false;
                for u in 0..self.num_vertices {
                    if self.removed[u] {
                        continue;
                    }
                    for j in self.csc.com[u]..self.csc.com[u + 1] {
                        let w = self.csc.unc[j];
                        if self.removed[w] {
                            continue;
                        }
                        if colors[u] > colors[w] {
                            colors[u] = colors[w];
                            color_changed = true;
                        }
                    }
                }
            }

            for v in 0..self.num_vertices {
                if !self.removed[v] && colors[v] == v {
                    self.bfs(colors[v], &colors);
                }
            }
        }
        log::trace!("Coloring scc took: {:?}", start.elapsed());
    }

    pub fn num_of_scc(&self) -> usize {
        self.scc_id
            .par_iter()
            .enumerate()
            .filter(|(i, id)| **id == *i)
            .count()
    }
}
