
# BYTE SAT

This is a simple C function that solves small SAT problems, specifically limited to 8 variables. The maximum number of clauses is not limited.

This technique exploits 256-bit bitvector manipulation to brute force through all 256 possible states of such a SAT instance. As a result, it only takes a few clock cycles to execute (provided your CPU has good vector support). Work scales linearly with the number of clauses, as the exponential factor is absorbed entirely by the vector width.

This approach could be scaled to larger SAT problems, but scales exponentially with the number of variables. So that's not recommended.


One nice thing about this technique is that it doesn't give you only a single satisfying state; rather it gives you a bitset corresponding to all satisfying states. If it gives you an array of just zeros, your SAT problem is unsatisfiable. No, you don't get a proof, that's too expensive. This is the cheap and dirty solution.

On my machine, this is able to run about 125 million clauses per second, though that number goes up by over a factor of 3 under certain circumstances, if the compiler can find a good way to vectorize multiple iterations together. This is roughly 8-28 clock cycles per clause.
