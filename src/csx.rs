use std::{
    fmt::{self, Formatter},
    fs::File,
    io::{self, BufRead},
    ops::Range,
    path::Path,
    time::Instant,
};

#[derive(Debug)]
#[allow(dead_code)]
pub struct CSX {
    pub num_of_vertices: usize,
    pub num_of_edges: usize,
    pub com: Vec<usize>,
    pub unc: Vec<usize>,
}

#[derive(Debug)]
pub enum CSXError {
    CouldNotOpenFile,
    BadMatrixFile,
    ErrorInHeader,
    ErrorInLine(usize),
}

impl std::error::Error for CSXError {}

impl fmt::Display for CSXError {
    fn fmt(&self, f: &mut Formatter) -> std::fmt::Result {
        match self {
            CSXError::CouldNotOpenFile => write!(f, "Could not open file."),
            CSXError::BadMatrixFile => write!(f, "Bad matrix file."),
            CSXError::ErrorInHeader => write!(f, "Error in header."),
            CSXError::ErrorInLine(l) => {
                write!(f, "There is an error in line {} after the header.", l + 1)
            }
        }
    }
}

impl CSX {
    pub fn new(num_of_vertices: usize, num_of_edges: usize) -> Self {
        CSX {
            num_of_vertices,
            num_of_edges,
            com: vec![num_of_edges; num_of_vertices + 1],
            unc: vec![0; num_of_edges],
        }
    }

    pub fn from_file(filename: &str) -> Result<Self, CSXError> {
        log::trace!("Reading matrix from file: {}", filename);
        let start = Instant::now();

        // open file
        let file = match File::open(Path::new(filename)) {
            Ok(f) => f,
            Err(_) => return Err(CSXError::CouldNotOpenFile),
        };
        let mut reader = io::BufReader::new(file);

        let mut line = String::new();

        // skip comments
        loop {
            match reader.read_line(&mut line) {
                Ok(0) => break,
                Ok(_) => {
                    if !line.starts_with("%") {
                        break;
                    }
                    line.clear();
                }
                Err(_) => return Err(CSXError::BadMatrixFile),
            }
        }

        // parse header
        let (m, nnz) = {
            let mut sizes = line
                .split_whitespace()
                .filter_map(|s| s.parse::<usize>().ok());

            let m = sizes.next().unwrap_or(0);
            let _n = sizes.next().unwrap_or(0);
            let nnz = sizes.next().unwrap_or(0);

            if m == 0 || _n == 0 || nnz == 0 {
                return Err(CSXError::ErrorInHeader);
            }

            (m, nnz)
        };
        line.clear();

        // convert file to csc format
        let mut csc = CSX::new(m, nnz);
        let mut com_i = 0;
        let mut line_number = 0;
        loop {
            match reader.read_line(&mut line) {
                Ok(0) => break,
                Err(_) => return Err(CSXError::BadMatrixFile),
                Ok(_) => {
                    let mut iter = line.split_whitespace();

                    fn get_next(line: Option<&str>, line_number: usize) -> Result<usize, CSXError> {
                        match line {
                            Some(s) => match s.parse::<usize>() {
                                Ok(i) => Ok(i),
                                Err(_) => Err(CSXError::ErrorInLine(line_number)),
                            },
                            None => Err(CSXError::ErrorInLine(line_number)),
                        }
                    }

                    // subtract 1 from value in file to convert to 0-based indexing
                    let a = match get_next(iter.next(), line_number) {
                        Ok(i) => i - 1,
                        Err(e) => return Err(e),
                    };

                    let b = match get_next(iter.next(), line_number) {
                        Ok(i) => i - 1,
                        Err(e) => return Err(e),
                    };

                    while com_i <= b {
                        csc.com[com_i] = line_number;
                        com_i += 1;
                    }

                    csc.unc[line_number] = a;
                }
            }
            line_number += 1;
            line.clear();
        }
        csc.com[com_i] = nnz;

        log::trace!("Finished reading file in {:?}", start.elapsed());
        return Ok(csc);
    }

    pub fn info(&self) {
        log::info!("{:_^20}", "Matrix Info");
        log::info!("M/N: {:>15}", self.num_of_vertices);
        log::info!("NNZ: {:>15}", self.num_of_edges);
    }

    pub fn range_for(&self, i: usize) -> Range<usize> {
        self.com[i]..self.com[i + 1]
    }
}
