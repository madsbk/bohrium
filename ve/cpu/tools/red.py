import numpy as np

def main(nelem, niter):
    """
    res = 0
    for _ in xrange(niter):
        res += np.sum(np.ones([nelem]*3) + np.arange(nelem**3).reshape([nelem]*3))/niter
    return res
    """

    #print np.sum(np.ones([nelem]*3)+np.arange(nelem**3).reshape([nelem]*3))/niter
    #print np.ones([nelem]*3) + np.arange(nelem**3).reshape([nelem]*3)
    #print 3*(1.0+np.sum(np.ones([nelem]*3) + np.arange(nelem**3).reshape([nelem]*3))/niter)
    #print 1.0+np.sum(np.ones([nelem]*3) + np.ones([nelem]*3))/niter
    #print 3*(1.0+np.add.reduce(np.ones([nelem]*3) + np.ones([nelem]*3))/niter)
    #print np.add.reduce(np.arange(nelem**3).reshape([nelem]*3), 1)
    #print np.add.accumulate(np.ones([nelem]*3))
    print np.add.accumulate(np.ones([nelem]*2))
    print np.add.accumulate(np.ones([nelem]*2),axis=1)

if __name__ == "__main__":
    print main(3, 3)
