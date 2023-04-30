use crate::{evil_pointer::EvilPtr, graph::Graph};

use rayon::prelude::*;
use std::time::Instant;

impl Graph {
    pub unsafe fn trim_par_unsafe(&mut self) -> std::io::Result<()> {
        let start = Instant::now();

        let mut has_in: Vec<bool> = vec![false; self.num_vertices];

        let mut has_out = vec![false; self.num_vertices];
        let has_out = EvilPtr::new(&mut has_out[0]);

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
                .filter_map(|unc| (*unc != idx).then(|| unsafe { has_out.deref().add(*unc) }));

            for ho in iter {
                *hi = true;
                unsafe { *ho = true };
            }
        });

        let trimmed = has_in
            .par_iter()
            .enumerate()
            .zip(self.removed.par_iter_mut())
            .map(|((i, hi), rm)| {
                if !*hi || !has_out[i] {
                    *rm = true;
                    1
                } else {
                    0
                }
            })
            .sum::<usize>();

        self.num_trimmed = trimmed;

        log::trace!("Trimmed {} vertices", self.num_trimmed);
        log::trace!("Trimming UNSAFE in parallel took: {:?}", start.elapsed());

        Ok(())
    }

    unsafe fn bfs_par_unsafe(
        &self,
        removed: &EvilPtr<bool>,
        scc_id: &EvilPtr<usize>,
        entry: usize,
        colors: &[usize],
    ) {
        *removed.deref().add(entry) = true;

        let mut fifo = Vec::new();

        for j in self.csc.com[entry]..self.csc.com[entry + 1] {
            fifo.push(self.csc.unc[j]);
        }

        let mut head = 0;

        while head != fifo.len() {
            let v = fifo[head];
            head += 1;

            if !removed[v] && colors[v] == entry {
                *removed.deref().add(v) = true;
                *scc_id.deref().add(v) = entry;

                for j in self.csc.com[v]..self.csc.com[v + 1] {
                    fifo.push(self.csc.unc[j]);
                }
            }
        }
    }

    pub unsafe fn color_scc_par_unsafe(&mut self) {
        log::trace!("Start coloring scc");
        let start = Instant::now();
        let mut colors = vec![0; self.num_vertices];
        let colors_ptr = EvilPtr::new(&mut colors[0]);

        while !self.is_empty_par() {
            colors
                .par_iter_mut()
                .enumerate()
                .for_each(|(v, color)| *color = self.scc_id[v]);

            let mut color_changed = true;
            let color_changed_ptr = EvilPtr::new(&mut color_changed);
            while color_changed {
                unsafe {
                    *color_changed_ptr.deref() = false;
                }

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
                            .for_each(|w| unsafe {
                                if *color > *colors_ptr.deref().add(*w) {
                                    *color = *colors_ptr.deref().add(*w);
                                    *color_changed_ptr.deref() = true;
                                }
                            });
                    });
            }

            let removed = EvilPtr::new(&mut self.removed[0]);
            let scc_id = EvilPtr::new(&mut self.scc_id[0]);

            self.removed
                .par_iter()
                .enumerate()
                .zip(colors.par_iter())
                .filter(|((i, r), color)| !**r && **color == *i)
                .for_each(|((_i, _r), color)| {
                    unsafe { self.bfs_par_unsafe(&removed, &scc_id, *color, &colors) };
                });
        }
        log::trace!(
            "Coloring scc UNSAFE in parallel took: {:?}",
            start.elapsed()
        );
    }
}
