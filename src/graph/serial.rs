use super::Graph;
use std::time::Instant;

impl Graph {
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

    pub fn is_empty(&self) -> bool {
        for v in 0..self.num_vertices {
            if !self.removed[v] {
                return false;
            }
        }
        return true;
    }

    pub fn bfs(&mut self, entry: usize, colors: &[usize]) {
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

    pub fn color_scc(&mut self) {
        log::trace!("Start coloring scc");
        let start = Instant::now();
        let mut colors = vec![0; self.num_vertices];

        while !self.is_empty() {
            colors
                .iter_mut()
                .enumerate()
                .for_each(|(i, c)| *c = self.scc_id[i]);

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
}
