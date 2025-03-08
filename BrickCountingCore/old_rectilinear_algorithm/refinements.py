import sys

S = {} # Symmetric refinements: string -> cnt
N = {} # Non-symmetric

# Set all A(X,X):
N['11'] = 22
S['11'] = 2
XX = '1'
XXs = 2
XXn = 22
for i in range(3,12):
    # An(X,X) = 46*An(X-1,X-1) + 6*2^X - 2^(X-1)
    # As(X,X) = 2 * As(X-1,X-1) = 2^(X-1)
    XX = XX + '1'
    S[XX] = XXs
    XXs = XXs * 2
    N[XX] = XXn
    XXn = 46*XXn + 6*2**i - 2**(i-1)

def setData(data):
    for i in range(0, len(data), 3):
        N[data[i]] = data[i+1] - data[i+2]
        S[data[i]] = data[i+2]

# Size 1:
setData(['1', 1, 1])

# Size 2:
# Done above

# Size 3:
setData(['21', 250, 20])

# Size 4:
setData(['31', 648, 8, '22', 10411, 49, '121', 37081, 32])

# Size 5:
setData(['41', 550, 28, '32', 148794, (443), '131', 433685, (24), '221', 1297413, (787)])

# Size 6:
setData(['51', 138, (4), '42', 849937, (473), '33', 6246077, (432)])
setData(['141', 2101339, (72), '321', 17111962, (671), '231', 41019966, (1179), '222', 43183164, (3305)])
setData(['1221', 157116243, (663)])

# Size 7:
setData(['61', 10, (4), '52', 2239070, (1788), '43', 106461697, (10551)])
setData(['421', 94955406, (6066), '241', 561350899, (15089), '331', 1358812234, (1104), '322', 561114147, (17838), '232', 3021093957, (46219), '151', 4940606, 12])
setData(['2221', 5227003593, (33392), '1321', 4581373745, (1471)])

# Size 8:
setData([
    '62', 2920534, (830),
    '53', 884147903, (5832),
    '44', 4297589646, (34099),
    '521', 245279996, (2456),
    '431', 20790340822, (23753),
    '422', 3125595194, (26862),
    '341', 41795025389, (17430),
    '332', 90630537410, (52944),
    '323', 7320657167, (14953),
    '251', 3894847047, (9174),
    '242', 84806603578, (143406),
    '161', 6059764, (12),
    '3221', 68698089712, (14219),
    '2321', 334184934526, (47632),
    '2231', 150136605052, (48678),
    '2222', 174623815718, (191947),
    '1421', 60442092848, (8871),
    '1331', 287171692047, (2640),
    '12221', 628137429871, (19191),
])

# Size 9:
setData([
    '72', 1989219, 1895,
    '63', 3968352541, 58092,
    '54', 82138898127,	281500,
    '621', 315713257,	10343,
    '522', 8147612224,	74040,
    '423', 41469827815,	143968,
    '261', 15217455035,	68536,
    '171', 4014751,	0,
    '4221', 392742794892,	301318,
    '3231', 1987600812703,	33113,
    '3222', 2312168563229,	759665,
])

# Impossible constructions:
setData(['91', 0, 0, '81', 0, 0, '71', 0, 0])


def get(prefix, A):
    if prefix in A:
        return A[prefix]
    if prefix[::-1] in A:
        return A[prefix[::-1]]
    if len(prefix) < 3:
        return None
    for i in range(1, len(prefix)-1):
        if prefix[i] == '1':
            prefixL = prefix[0:i+1:]
            left = get(prefixL, A)
            prefixR = prefix[i::]
            right = get(prefixR, A)
            if left != None and right != None:
                return [prefixL, prefixR]

def getS(prefix):
    x = get(prefix, S)
    if x == None or type(x) is int:
        return x
    cs = get(x[0], S)
    ds = get(x[1], S)
    S[prefix] = cs * ds
    return cs * ds

def getN(prefix):
    x = get(prefix, N)
    if x == None or type(x) is int:
        return x
    cn = get(x[0], N)
    cs = get(x[0], S)
    dn = get(x[1], N)
    ds = get(x[1], S)
    d = dn+ds
    ret = (dn+d)*cn + d*cs - cs*ds
    N[prefix] = ret
    return ret


printed = {}
def printXY(X, Y, prefix, rem, actuallyPrint):
    if rem == 0 and len(prefix) == Y:
        s = getS(prefix)
        n = getN(prefix)
        sum = [0,0] if None in [s,n] else [n,s]
        if prefix in printed:
            return sum # Already printed: Should still be counted
        printed[prefix] = True
        printed[prefix[::-1]] = True
        if n == 0 and s == 0 or not actuallyPrint:
            return sum
        if n != None and s != None:
            print('  <' + prefix + '>', s + getN(prefix), '(' + str(s) + ')')
            printed[prefix[::-1]] = True
        else:
            print('  <' + prefix + '> #')
        return sum
    if len(prefix) >= Y:
        return (0,0) # Too tall!
    sum = [0,0]
    for c in range(min(9, rem), 0, -1):
        add = printXY(X, Y, prefix + str(c), rem-c, actuallyPrint)
        sum[0] = sum[0] + add[0]
        sum[1] = sum[1] + add[1]
    return sum

to = int(sys.argv[1])
print()
print('Refinements of size', to)
for X in range(2,to+1):
    sumN = 0
    sumS = 0
    for Y in range(2,X+1):
        if X == to:
            print(' Height', Y)
        [n,s] = printXY(X, Y, '', X, X == to)
        sumN = sumN + n
        sumS = sumS + s
        if X == to:
            print('   SUM', n+s, '(' + str(s) + ')')
    if X == to:
        print('TOTAL', sumN+sumS, '(' + str(sumS) + ')')


# Code below recreates Table 7 from Eilers (2016):
def prep():
    return [[0 for x in range(to)] for y in range(to)]
f = [0] * to # c with line on top in Table 7
f180 = [0] * to
C = prep() # c(n,m) with bold c
C90 = prep()
if to >= 8:
    C90[7][3] = 244
C180 = prep()

def fillC(n, prefix, rem):
    if rem == 0 and len(prefix) > 0:
        if prefix[0] == '1':
            # f and f180:
            fprefix = prefix + '1'
            N = getN(fprefix)
            S = getS(fprefix)
            if N is not None:
                f[n-1] = f[n-1] + 2 * N + S
                f180[n-1] = f180[n-1] + getS(fprefix)
        # C and C180:
        firstLayer = int(prefix[0])
        N = getN(prefix)
        S = getS(prefix)        
        if N is not None:
            C[n-1][firstLayer-1] = C[n-1][firstLayer-1] + (2 * N + S) * firstLayer
            C180[n-1][firstLayer-1] = C180[n-1][firstLayer-1] + S * firstLayer
        return
    for m in range(1 if prefix == '' else 2, rem+1):
        fillC(n+m, prefix + str(m), rem-m)

for n in range(1, to+1):
    fillC(0, '', n)

print('n\tf(n)\tf180(n)\t' + '\t'.join(["C(n,{0})\tC180(n,{0})".format(x) for x in range(1,to+1)]))
for n in range(0, to):
    printList = [n+1,f[n],f180[n]]
    for m in range(0, to):
        printList.append(C[n][m])
        printList.append(C180[n][m])
    print('\t'.join([str(x) for x in printList]))

def multf(remainingCount, remainingN, product, ret, go180):
    if remainingCount == 0:
        if remainingN != 0:
            return
        ret.append(product);
        return
    for i in range(1, remainingN+1):
        multf(remainingCount-1, remainingN-i, product * (f180[i-1] if go180 else f[i-1]), ret, go180)
    
# Compute a(n) using 2.3 from Eilers (2016):
for n in range(1, to+1):
    an = 0
    # First row sum:
    for m in range(2, n+1):
        an = an + (C[n-1][m-1] + C180[n-1][m-1] + 2 * C90[n-1][m-1]) / (2 * m)
    # Second row first sum:
    suml = 0
    for l in range(n): # l = 0...n-1
        for m1 in range(1,n+1): # m1 = 1..n
            for m2 in range(1,n+2-m1): # m2 = 1..
                sums = []
                multf(l, n+1-m1-m2, 1, sums, False)
                for sum in sums:
                    suml = suml + C[m1-1][0] * C[m2-1][0] * sum
                sums180 = []
                multf(l, n+1-m1-m2, 1, sums180, True)
                for sum in sums180:
                    suml = suml + C180[m1-1][0] * C180[m2-1][0] * sum
    an = an + suml / 2
    print('a(' + str(n) + ') =', an)
