#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/list.h>

asmlinkage long sys_hello(void){
	printk("Hello World!\n");
	return 0;
}

asmlinkage long sys_set_weight(int given_weight){
	if (given_weight < 0){
		return -EINVAL;
	}
	current->weight = given_weight;
	return 0;
}
asmlinkage long sys_get_weight(void){
	return current->weight;
}

asmlinkage long sys_get_ancestor_sum(void){
	int sum = 0;
	struct task_struct *p = current;
	if (current->pid == 1){
		return current->weight;
	}
	while (p->pid != 1)
	{
		sum += p->weight;
		p = p->parent;
	}
	sum += p->weight;
	return sum;
}

int has_children(struct task_struct *b){
	struct task_struct *a;
	list_for_each_entry(a, &b->children, sibling){
		return 1;
	}
	return 0;
}

int recur_heaviest(struct task_struct *k){
	struct task_struct *c;
	int max = k->weight;
	pid_t pid_hd = k->pid;
	int temp_max;
	list_for_each_entry(c, &k->children, sibling){
		if (has_children(c)){
			temp_max = recur_heaviest(c);
			if (temp_max > max){
				max = temp_max;
				pid_hd = c->pid_of_hd;
			}
			else{
				if (temp_max == max){
					if (c->pid_of_hd < pid_hd){
						pid_hd = c->pid_of_hd;
					}
				}
			}
		}
		else{
			if (c->weight > max){
				max = c->weight;
				pid_hd = c->pid;
			}
			else{
				if (c->weight == max){
					if (c->pid < pid_hd){
						pid_hd = c->pid;
					}
				}
			}
		}
	}
	k->pid_of_hd = pid_hd;
	return max;
}

asmlinkage long sys_get_heaviest_descendant(void){
	if (!(has_children(current))){
		return -ECHILD;
	}
	struct task_struct *c;
	int max = -1;
	int temp_max = -1;
	pid_t pid_of_winner = -1;
	list_for_each_entry(c, &current->children, sibling){
		temp_max = recur_heaviest(c);
		if (temp_max > max){
			max = temp_max;
			pid_of_winner = c->pid_of_hd;
		}
		else{
			if (temp_max == max){
				if (c->pid_of_hd < pid_of_winner){
					pid_of_winner = c->pid_of_hd;
				}
			}
		}
	}
	return pid_of_winner;
}
