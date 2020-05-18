int gcd(int a, int b){ return b ?gcd(b, a%b) :a;}
