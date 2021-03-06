#include "cpu/exec.h"

/* PA 3.1 */
make_EHelper(lidt) {

        cpu.idtr.limit = vaddr_read(id_dest->addr, 2);
        cpu.idtr.base = decinfo.isa.is_operand_size_16 ? vaddr_read(id_dest->addr + 2, 4) \
            && 0x00ffffff : vaddr_read(id_dest->addr + 2, 4);

        print_asm_template1(lidt);
}

make_EHelper(mov_r2cr) {
        TODO();

        print_asm("movl %%%s,%%cr%d", reg_name(id_src->reg, 4), id_dest->reg);
}

make_EHelper(mov_cr2r) {
        TODO();

        print_asm("movl %%cr%d,%%%s", id_src->reg, reg_name(id_dest->reg, 4));

        difftest_skip_ref();
}

make_EHelper(int) {
    /* PA 3.1*/
    raise_intr(id_dest->val, decinfo.seq_pc);

    print_asm("int %s", id_dest->str);

    difftest_skip_dut(1, 2);
}

/* PA 3.1 */
make_EHelper(iret) {
        rtl_pop(&decinfo.jmp_pc);
        rtl_pop(&cpu.cs);
        rtl_pop(&cpu.eflags_value);

        rtl_j(decinfo.jmp_pc);

        print_asm("iret");
}

uint32_t pio_read_l(ioaddr_t);

uint32_t pio_read_w(ioaddr_t);

uint32_t pio_read_b(ioaddr_t);

void pio_write_l(ioaddr_t, uint32_t);

void pio_write_w(ioaddr_t, uint32_t);

void pio_write_b(ioaddr_t, uint32_t);

/* PA2.3 */
make_EHelper(in) {
        //TODO();
        switch (id_src->width){
            case 1:
                s0 = pio_read_b(id_src->val);
            break;
            case 2:
                s0 = pio_read_w(id_src->val);
            break;
            case 4:
                s0 = pio_read_l(id_src->val);
            break;
            default:
                break;
        }
        operand_write(id_dest, &s0);

        print_asm_template2(in);
}

make_EHelper(out) {
        switch (id_src->width){
            case 1:
                pio_write_b(id_dest->val, id_src->val);
            break;
            case 2:
                pio_write_w(id_dest->val, id_src->val);
            break;
            case 4:
                pio_write_l(id_dest->val, id_src->val);
            break;
            default:
                break;
        }

        print_asm_template2(out);
}
