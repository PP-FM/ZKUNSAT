
class TestCase:
    def __init__(self, nres, ncls, degree, res_time, access_time, input_time, total_time):
        self.nres = nres
        self.ncls = ncls
        self.degree = degree
        self.res_time = res_time
        self.access_time = access_time
        self.total_time = total_time
        self.input_time = input_time
    def __repr__(self):
        res = ""
        res += "ncls:{}, ".format(self.ncls)
        res += "nres:{}, ".format(self.nres)
        res += "degree:{}, ".format(self.degree)
        return res

def data_process(filename):
    results = []
    with open(filename) as f:
        while (True):
            # Read a line.
            line = f.readline()

            if len(line) == 0:
                print("::DONE::")
                break
            # When a newline is returned, the line is empty.
            if line == "----set up----\n":
                res = f.readline()
                nres = int(res.split()[1])
                cls = f.readline()
                ncls = int(cls.split()[1])
                d = f.readline()
                degree = int(d.split()[1])
                time = f.readline()
                while (time.split()[0]!= "a"):
                    time = f.readline()
                # print(nres, ncls, degree)
                access_time = float(time.split()[1])
                res_time = float(time.split()[3])
                input_time = float(time.split()[5])
                total_time = float(time.split()[7])

                tc = TestCase(nres, ncls, degree, res_time, access_time, input_time, total_time)
                results.append(tc)
    return results
#
# import matplotlib.pyplot as plt
# bi_det = "results_det.txt"
# bi_random= "results_rand.txt"
# results = data_process(bi_det)
#
# def i_forget():
#     s = set()
#     for tc in results:
#         s.add(tc.nvar)
#     print(sorted(s))
#
# i_forget()
# s = set()
# for tc in results:
#     s.add(tc.ncls)
# print(sorted(s))



import matplotlib.pyplot as plt
import  numpy as np
import  math
marker = ['-.', '-', '--', '-p', '-v', '-+', '-X', '-*' ]
colors = ["#332288", "#88CCEE", "#44AA99", "#117733", "#999933", "#DDCC77"]
def init_plotting(fig_width = 8, fig_height = 0, font= 15):
    golden_mean = (math.sqrt(5)-1.0)/2.0    # Aesthetic ratio
    if fig_height == 0:
        fig_height = fig_width*golden_mean # height in inches
    plt.rcParams['text.usetex'] = True
    plt.rcParams['figure.figsize'] =[fig_width,fig_height]
    plt.rcParams['font.size'] = font
    #plt.rcParams['font.family'] = 'Times New Roman'
    plt.rcParams['font.family'] = 'serif'
    plt.rcParams['font.serif'] = 'Times'
    plt.rcParams['axes.labelsize'] = plt.rcParams['font.size']
    plt.rcParams['axes.titlesize'] = 1.5*plt.rcParams['font.size']
    plt.rcParams['legend.fontsize'] = plt.rcParams['font.size']
    plt.rcParams['xtick.labelsize'] = plt.rcParams['font.size']
    plt.rcParams['ytick.labelsize'] = plt.rcParams['font.size']

init_plotting()

width = 0.15
half_width = width /2
fig, ax = plt.subplots(figsize=(8,5))
labels = ["$R$ = 2000", "$R$ = 4000", "$R$ = 6000", "$R$ = 8000"]
x = np.arange(len(labels))


results = data_process("./micro.result")

##############################



access_time = [tc.access_time for tc in results if  ( tc.ncls -tc.nres-1 == 3000  and tc.nres == 2000 and tc.degree == 100) or ( tc.ncls -tc.nres-1 == 3000  and tc.nres == 4000 and tc.degree == 100)  or (tc.ncls -tc.nres-1 == 3000  and tc.nres == 6000 and tc.degree == 100) or (tc.ncls -tc.nres-1 == 3000  and tc.nres == 8000 and tc.degree == 100) ]

res_time =  [tc.res_time for tc in results if  (tc.ncls -tc.nres-1 == 3000  and tc.nres == 2000 and tc.degree == 100)  or (tc.ncls -tc.nres-1 == 3000  and tc.nres == 4000 and tc.degree == 100)  or (tc.ncls -tc.nres-1 == 3000  and tc.nres == 6000 and tc.degree == 100) or (tc.ncls -tc.nres-1 == 3000  and tc.nres == 8000 and tc.degree == 100) ]

input_time =  [tc.input_time for tc in results if (tc.ncls -tc.nres-1 == 3000  and tc.nres == 2000 and tc.degree == 100) or (tc.ncls -tc.nres-1 == 3000  and tc.nres == 4000 and tc.degree == 100)  or (tc.ncls -tc.nres-1 == 3000  and tc.nres == 6000 and tc.degree == 100) or (tc.ncls -tc.nres-1 == 3000  and tc.nres == 8000 and tc.degree == 100) ]

total_time =  [tc.total_time for tc in results if (tc.ncls -tc.nres-1 == 3000  and tc.nres == 2000 and tc.degree == 100) or  (tc.ncls -tc.nres-1 == 3000  and tc.nres == 4000 and tc.degree == 100)  or (tc.ncls -tc.nres-1 == 3000  and tc.nres == 6000 and tc.degree == 100) or (tc.ncls -tc.nres-1 == 3000  and tc.nres == 8000 and tc.degree == 100) ]




ax.bar(x - 3* half_width, input_time, width, label='Input',  edgecolor='black', color = 'pink')
total = input_time

ax.bar(x - 3* half_width,  access_time, width,  bottom=total, label='Access', hatch = '\\',  edgecolor='black',  color = 'lightskyblue')
total = [x + y for x,y in zip(access_time, total)]

rect = ax.bar(x - 3* half_width,  res_time, width,  bottom=total, label='Resolve', hatch = '/',  edgecolor='black',  color = 'bisque')
total = [x + y for x,y in zip(res_time, total)]

case = ["$D_1$", "$D_1$", "$D_1$", "$D_1$"]
for height, rect, label in zip(total, rect, case):
    ax.text(rect.get_x() + rect.get_width()/2, height + 0.1, label,  ha='center', va='bottom',)

#################

access_time = [tc.access_time for tc in results if (tc.ncls -tc.nres-1 == 3000  and tc.nres == 2000 and tc.degree == 200) or  (tc.ncls -tc.nres-1 == 3000  and tc.nres == 4000 and tc.degree == 200)  or (tc.ncls -tc.nres-1 == 3000  and tc.nres == 6000 and tc.degree == 200) or (tc.ncls -tc.nres-1 == 3000  and tc.nres == 8000 and tc.degree == 200) ]

res_time =  [tc.res_time for tc in results if (tc.ncls -tc.nres-1 == 3000  and tc.nres == 2000 and tc.degree == 200) or (tc.ncls -tc.nres-1 == 3000  and tc.nres == 4000 and tc.degree == 200)  or (tc.ncls -tc.nres-1 == 3000  and tc.nres == 6000 and tc.degree == 200) or (tc.ncls -tc.nres-1 == 3000  and tc.nres == 8000 and tc.degree == 200) ]

input_time =  [tc.input_time for tc in results if (tc.ncls -tc.nres-1 == 3000  and tc.nres == 2000 and tc.degree == 200) or (tc.ncls -tc.nres-1 == 3000  and tc.nres == 4000 and tc.degree == 200)  or (tc.ncls -tc.nres-1 == 3000  and tc.nres == 6000 and tc.degree == 200) or (tc.ncls -tc.nres-1 == 3000  and tc.nres == 8000 and tc.degree == 200) ]

total_time =  [tc.total_time for tc in results if (tc.ncls -tc.nres-1 == 3000  and tc.nres == 2000 and tc.degree == 200) or (tc.ncls -tc.nres-1 == 3000  and tc.nres == 4000 and tc.degree == 200)  or (tc.ncls -tc.nres-1 == 3000  and tc.nres == 6000 and tc.degree == 200) or (tc.ncls -tc.nres-1 == 3000  and tc.nres == 8000 and tc.degree == 200) ]



ax.bar(x  - half_width, input_time, width,   edgecolor='black', color = 'pink')
total = input_time

ax.bar(x  - half_width,  access_time, width,  bottom=total, hatch = '\\',  edgecolor='black',  color = 'lightskyblue')
total = [x + y for x,y in zip(access_time, total)]

rect = ax.bar(x  - half_width,  res_time, width,  bottom=total,  hatch = '/',  edgecolor='black',  color = 'bisque')
total = [x + y for x,y in zip(res_time, total)]

case = ["$D_2$",  "$D_2$", "$D_2$", "$D_2$"]
for height, rect, label in zip(total, rect, case):
    ax.text(rect.get_x() + rect.get_width()/2, height + 0.1, label,  ha='center', va='bottom',)

#################

access_time = [tc.access_time for tc in results if (tc.ncls -tc.nres-1 == 3000  and tc.nres == 2000 and tc.degree == 300)  or (tc.ncls -tc.nres-1 == 3000  and tc.nres == 4000 and tc.degree == 300)  or (tc.ncls -tc.nres-1 == 3000  and tc.nres == 6000 and tc.degree == 300) or (tc.ncls -tc.nres-1 == 3000  and tc.nres == 8000 and tc.degree == 300) ]

res_time =  [tc.res_time for tc in results if (tc.ncls -tc.nres-1 == 3000  and tc.nres == 2000 and tc.degree == 300)  or (tc.ncls -tc.nres-1 == 3000  and tc.nres == 4000 and tc.degree == 300)  or (tc.ncls -tc.nres-1 == 3000  and tc.nres == 6000 and tc.degree == 300) or (tc.ncls -tc.nres-1 == 3000  and tc.nres == 8000 and tc.degree == 300) ]

input_time =  [tc.input_time for tc in results if (tc.ncls -tc.nres-1 == 3000  and tc.nres == 2000 and tc.degree == 300)  or (tc.ncls -tc.nres-1 == 3000  and tc.nres == 4000 and tc.degree == 300)  or (tc.ncls -tc.nres-1 == 3000  and tc.nres == 6000 and tc.degree == 300) or (tc.ncls -tc.nres-1 == 3000  and tc.nres == 8000 and tc.degree == 300) ]

total_time =  [tc.total_time for tc in results if(tc.ncls -tc.nres-1 == 3000  and tc.nres == 2000 and tc.degree == 300)  or  (tc.ncls -tc.nres-1 == 3000  and tc.nres == 4000 and tc.degree == 300)  or (tc.ncls -tc.nres-1 == 3000  and tc.nres == 6000 and tc.degree == 300) or (tc.ncls -tc.nres-1 == 3000  and tc.nres == 8000 and tc.degree == 300) ]



ax.bar(x +  half_width, input_time, width,   edgecolor='black', color = 'pink')
total = input_time

ax.bar(x +  half_width,  access_time, width,  bottom=total, hatch = '\\',  edgecolor='black',  color = 'lightskyblue')
total = [x + y for x,y in zip(access_time, total)]

rect = ax.bar(x +  half_width,  res_time, width,  bottom=total,  hatch = '/',  edgecolor='black',  color = 'bisque')
total = [x + y for x,y in zip(res_time, total)]

case = ["$D_3$", "$D_3$", "$D_3$", "$D_3$"]
for height, rect, label in zip(total, rect, case):
    ax.text(rect.get_x() + rect.get_width()/2, height + 0.1, label,  ha='center', va='bottom',)

###################################

access_time = [tc.access_time for tc in results if  (tc.ncls -tc.nres-1 == 3000  and tc.nres == 2000 and tc.degree == 400)  or (tc.ncls -tc.nres-1 == 3000  and tc.nres == 4000 and tc.degree == 400)  or (tc.ncls -tc.nres-1 == 3000  and tc.nres == 6000 and tc.degree == 400) or (tc.ncls -tc.nres-1 == 3000  and tc.nres == 8000 and tc.degree == 400) ]

res_time =  [tc.res_time for tc in results if  (tc.ncls -tc.nres-1 == 3000  and tc.nres == 2000 and tc.degree == 400)  or  (tc.ncls -tc.nres-1 == 3000  and tc.nres == 4000 and tc.degree == 400)  or (tc.ncls -tc.nres-1 == 3000  and tc.nres == 6000 and tc.degree == 400) or (tc.ncls -tc.nres-1 == 3000  and tc.nres == 8000 and tc.degree == 400) ]

input_time =  [tc.input_time for tc in results if (tc.ncls -tc.nres-1 == 3000  and tc.nres == 2000 and tc.degree == 400)  or (tc.ncls -tc.nres-1 == 3000  and tc.nres == 4000 and tc.degree == 400)  or (tc.ncls -tc.nres-1 == 3000  and tc.nres == 6000 and tc.degree == 400) or (tc.ncls -tc.nres-1 == 3000  and tc.nres == 8000 and tc.degree == 400) ]

total_time =  [tc.total_time for tc in results if  (tc.ncls -tc.nres-1 == 3000  and tc.nres == 2000 and tc.degree == 400)  or (tc.ncls -tc.nres-1 == 3000  and tc.nres == 4000 and tc.degree == 400)  or (tc.ncls -tc.nres-1 == 3000  and tc.nres == 6000 and tc.degree == 400) or (tc.ncls -tc.nres-1 == 3000  and tc.nres == 8000 and tc.degree == 400) ]



ax.bar(x + 3* half_width, input_time, width,   edgecolor='black', color = 'pink')
total = input_time

ax.bar(x + 3* half_width,  access_time, width,  bottom=total, hatch = '\\',  edgecolor='black',  color = 'lightskyblue')
total = [x + y for x,y in zip(access_time, total)]

rect = ax.bar(x + 3 * half_width,  res_time, width,  bottom=total,  hatch = '/',  edgecolor='black',  color = 'bisque')
total = [x + y for x,y in zip(res_time, total)]

case = ["$D_4$", "$D_4$", "$D_4$", "$D_4$"]
for height, rect, label in zip(total, rect, case):
    ax.text(rect.get_x() + rect.get_width()/2, height + 0.1, label,  ha='center', va='bottom',)



props = dict(boxstyle='round', facecolor='w', alpha=0.5)

textstr = "$D_1: D = 100$ \n$D_2: D = 200$ \n$D_3: D = 300$\n$D_4: D = 400$"
ax.text(0.05, 0.95, textstr, transform=ax.transAxes, fontsize=10,
        verticalalignment='top', bbox=props)

ax.set_xticks(x)
ax.set_xticklabels(labels)

ax.set_ylabel ('Time (s)')
ax.legend(loc='upper center', bbox_to_anchor=(0.5, -0.05),
          ncol=5, fancybox=True, shadow= False)

props = dict(boxstyle='round', facecolor='w', alpha=0.5)


fig.savefig('./microbenchmark_resolution_chart.pdf', bbox_inches='tight', dpi=200)
plt.show()


