<script>
    import { toasts } from "../lib/toast.js";
</script>

<div class="fixed top-3 right-3 space-y-2 z-50">
    {#each $toasts as t}
        <div
            class="bg-slate-800/90 border border-slate-700 rounded px-3 py-2 shadow text-sm"
        >
            {t.text}
        </div>
    {/each}
</div>
