use super::Graph;

use rayon::prelude::*;
use std::{
    sync::atomic::{AtomicBool, AtomicUsize, Ordering},
    time::Instant,
};

impl Graph {
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

    pub fn is_empty_par(&self) -> bool {
        !self.removed.par_iter().any(|removed| !removed)
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
            colors.par_iter_mut().enumerate().for_each(|(v, color)| {
                *color = self.scc_id[v];
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
}
