import matplotlib.pyplot as plt

# 提取的数据
sizes = [1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000]
l1_i_misses = [35, 31, 35, 666, 3058, 20394, 121578]
l1_d_misses = [112, 93, 111, 1214, 15281, 89890, 905258]
instructions = [12694, 111694, 1101694, 11042854, 110540132, 1103550600, 11036612698]
cycles = [64909, 533230, 5213107, 52398179, 522681956, 5214899819, 52164517983]

# ---- 全局样式设置 ----
# 让字体和线条看起来更柔和现代
plt.rcParams['font.family'] = 'sans-serif'
plt.rcParams['axes.linewidth'] = 1.2
text_color = '#333333'

# 提取出一个通用的坐标轴美化函数
def setup_beautiful_axes(ax, title, ylabel):
    ax.set_title(title, fontsize=15, fontweight='bold', pad=15, color=text_color)
    ax.set_xlabel('Input Size (N)', fontsize=12, fontweight='bold', color='#555555')
    ax.set_ylabel(ylabel, fontsize=12, fontweight='bold', color='#555555')
    
    ax.set_xscale('log')
    ax.set_yscale('log')
    
    # 精致的网格线：主线明显，副线虚化
    ax.grid(True, which='major', linestyle='--', linewidth=0.8, color='#CCCCCC')
    ax.grid(True, which='minor', linestyle=':', linewidth=0.5, color='#EEEEEE')
    
    # 去除顶部和右侧的边框，更符合现代审美
    ax.spines['top'].set_visible(False)
    ax.spines['right'].set_visible(False)
    ax.spines['left'].set_color('#888888')
    ax.spines['bottom'].set_color('#888888')

# ==========================================
# 第一张图：Cache Misses
# ==========================================
fig1, ax1 = plt.subplots(figsize=(9, 6), dpi=150) # 提高分辨率
setup_beautiful_axes(ax1, 'Evolution of L1 Cache Misses', 'Number of Misses (Log Scale)')

# L1-D Misses: 蓝色，圆形标记带白边
ax1.plot(sizes, l1_d_misses, color='#1f77b4', linewidth=2.5, 
         marker='o', markersize=9, markerfacecolor='#1f77b4', 
         markeredgecolor='white', markeredgewidth=1.5, label='L1-D Misses', zorder=3)

# L1-I Misses: 橙色，方形标记带白边
ax1.plot(sizes, l1_i_misses, color='#ff7f0e', linewidth=2.5, 
         marker='s', markersize=8, markerfacecolor='#ff7f0e', 
         markeredgecolor='white', markeredgewidth=1.5, label='L1-I Misses', zorder=3)

# 漂亮的图例
ax1.legend(loc='upper left', frameon=True, shadow=True, fancybox=True, fontsize=11, borderpad=1)
fig1.tight_layout()
fig1.savefig('cache_misses_pro.png', bbox_inches='tight')


# ==========================================
# 第二张图：Instructions & Cycles
# ==========================================
fig2, ax2 = plt.subplots(figsize=(9, 6), dpi=150)
setup_beautiful_axes(ax2, 'Instructions and Cycles vs. Input Size', 'Count (Log Scale)')

# Total Cycles: 红色，三角形标记带白边
ax2.plot(sizes, cycles, color='#d62728', linewidth=2.5, 
         marker='^', markersize=10, markerfacecolor='#d62728', 
         markeredgecolor='white', markeredgewidth=1.5, label='Total Cycles', zorder=3)

# Total Instructions: 绿色，菱形标记带白边
ax2.plot(sizes, instructions, color='#2ca02c', linewidth=2.5, 
         marker='D', markersize=8, markerfacecolor='#2ca02c', 
         markeredgecolor='white', markeredgewidth=1.5, label='Total Instructions', zorder=3)

ax2.legend(loc='upper left', frameon=True, shadow=True, fancybox=True, fontsize=11, borderpad=1)
fig2.tight_layout()
fig2.savefig('instructions_cycles_pro.png', bbox_inches='tight')

# 显示图片
plt.show()