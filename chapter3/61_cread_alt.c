long cread(long *xp) {
    return (xp ? *xp : 0);
}

long cread_alt(long *xp) {
    long zero = 0;
    long *valid_pointer = (xp ? xp : &zero);
    return *valid_pointer;
}

