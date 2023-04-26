use std::ops::Index;

pub struct EvilPtr<T> {
    ptr: *mut T,
}

impl<T> EvilPtr<T> {
    pub fn new(data: &mut T) -> Self {
        Self { ptr: data }
    }

    pub unsafe fn deref(&self) -> *mut T {
        self.ptr
    }
}

impl<T> Index<usize> for EvilPtr<T> {
    type Output = T;

    fn index(&self, index: usize) -> &Self::Output {
        unsafe { &*self.ptr.add(index) }
    }
}

unsafe impl<T> Sync for EvilPtr<T> {}
unsafe impl<T> Send for EvilPtr<T> {}
