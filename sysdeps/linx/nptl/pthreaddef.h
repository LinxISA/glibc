/* pthread machine parameter definitions, LinxISA bring-up version.
   This file is part of the GNU C Library.
 */

/* Default stack size.  */
#define ARCH_STACK_DEFAULT_SIZE (2 * 1024 * 1024)

/* Minimum guard size.  */
#define ARCH_MIN_GUARD_SIZE 0

/* Required stack pointer alignment at beginning.  */
#define STACK_ALIGN 16

/* Minimal stack size after allocating thread descriptor and guard size.  */
#define MINIMAL_REST_STACK 2048

/* Location of current stack frame.  */
#define CURRENT_STACK_FRAME __builtin_frame_address (0)

