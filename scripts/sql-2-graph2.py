import sqlite3
import sys

# Typical packages
import sys
from pathlib import Path

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib
matplotlib.style.use('ggplot')


def main(path: Path):
    df1 = pd.DataFrame(10 * np.random.rand(4, 3), index=["A", "B", "C", "D"], columns=["I", "J", "K"])
    df2 = pd.DataFrame(10 * np.random.rand(4, 3), index=["A", "B", "C", "D"], columns=["I", "J", "K"])
    df3 = pd.DataFrame(10 * np.random.rand(4, 3), index=["A", "B", "C", "D"], columns=["I", "J", "K"])

    def prep_df(df, name):
        df = df.stack().reset_index()
        df.columns = ['c1', 'c2', 'values']
        df['DF'] = name
        return df

    df1 = prep_df(df1, 'DF1')
    df2 = prep_df(df2, 'DF2')
    df3 = prep_df(df3, 'DF3')

    df = pd.concat([df1, df2, df3])
    print(df.index)

    fig, axes = plt.subplots(nrows=1, ncols=3)

    ax_position = 0
    for concept in df.index.get_level_values('concept').unique():
        idx = pd.IndexSlice
        subset = df.loc[idx[[concept], :],
                        ['cmp_tr_neg_p_wrk', 'exp_tr_pos_p_wrk',
                         'cmp_p_spot', 'exp_p_spot']]
        print(subset.info())
        subset = subset.groupby(
            subset.index.get_level_values('datetime').year).sum()
        subset = subset / 4  # quarter hours
        subset = subset / 100  # installed capacity
        ax = subset.plot(kind="bar", stacked=True, colormap="Blues",
                         ax=axes[ax_position])
        ax.set_title("Concept \"" + concept + "\"", fontsize=30, alpha=1.0)
        ax.set_ylabel("Hours", fontsize=30),
        ax.set_xlabel("Concept \"" + concept + "\"", fontsize=30, alpha=0.0),
        ax.set_ylim(0, 9000)
        ax.set_yticks(range(0, 9000, 1000))
        ax.set_yticklabels(labels=range(0, 9000, 1000), rotation=0,
                           minor=False, fontsize=28)
        ax.set_xticklabels(labels=['2012', '2013', '2014'], rotation=0,
                           minor=False, fontsize=28)
        handles, labels = ax.get_legend_handles_labels()
        ax.legend(['Market A', 'Market B',
                   'Market C', 'Market D'],
                  loc='upper right', fontsize=28)
        ax_position += 1

    # look "three subplots"
    # plt.tight_layout(pad=0.0, w_pad=-8.0, h_pad=0.0)

    # look "one plot"
    plt.tight_layout(pad=0., w_pad=-16.5, h_pad=0.0)
    axes[1].set_ylabel("")
    axes[2].set_ylabel("")
    axes[1].set_yticklabels("")
    axes[2].set_yticklabels("")
    axes[0].legend().set_visible(False)
    axes[1].legend().set_visible(False)
    axes[2].legend(['Market A', 'Market B',
                    'Market C', 'Market D'],
                   loc='upper right', fontsize=28)


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("usage: {} <database_path>".format(sys.argv[0]))
        exit(1)
    main(Path(sys.argv[1]))
