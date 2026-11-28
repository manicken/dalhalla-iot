/**
 * modal.js — generic movable modal window.
 *
 * Completely decoupled from what goes inside it. Give it a title and,
 * optionally, a body node — it just provides window chrome. Reuse it for
 * anything: the REGO600 panel, a WiFi settings form, a log viewer, etc.
 *
 * Two chrome styles, both from the same class:
 *   - Free-floating tool window (default): draggable, no backdrop, can
 *     stay open alongside other UI. Used by the REGO600 panel.
 *   - Blocking dialog: `backdrop: true` adds a dimmed overlay, click-outside
 *     and Escape both close it, and dialog ARIA attributes are set
 *     automatically. Typically paired with `draggable: false`. Used by
 *     the WiFi setup form.
 *
 * Usage:
 *   const modal = new Modal({ title: "REGO600 Control Panel" });
 *   modal.setBody(someElement);
 *   modal.setFooter(someButtonRow);  // optional
 *   modal.mount();   // attaches to document.body
 *   modal.open();
 */
class Modal {
	constructor({
		id,
		title = "",
		width,
		height,
		x,
		y,
		closable = true,
		draggable = true,
		backdrop = false,
		closeOnBackdropClick = true,
		closeOnEscape = backdrop,
		onClose,
		onOpen,
	} = {}) {
		this.id = id || `modal-${Math.random().toString(36).slice(2, 9)}`;
		this.onClose = onClose;
		this.onOpen = onOpen;
		this.hasBackdrop = backdrop;

		this.el = newEl("div", { cls: "modal", id: this.id, styles: { width: width ? `${width}px` : "", height: height ? `${height}px` : "" } });
		this.titleEl = newEl("div", { cls: "modal-title", id: `${this.id}-title`, text: title });
		this.headerEl = newEl("div", { cls: "modal-header" }, this.titleEl);

		if (backdrop) {
			this.el.setAttribute("role", "dialog");
			this.el.setAttribute("aria-modal", "true");
			this.el.setAttribute("aria-labelledby", this.titleEl.id);
		}

		if (closable) {
			this.closeBtn = newEl("button", { cls: "modal-close", type: "button", "aria-label": "Close", text: "\u00d7", onclick: () => this.close() });
			this.headerEl.append(this.closeBtn);
		}

		this.bodyEl = newEl("div", { cls: "modal-body" });
		this.footerEl = newEl("div", { cls: "modal-footer", styles: { display: "none" } }); // hidden until setFooter() is used
		this.el.append(this.headerEl, this.bodyEl, this.footerEl);

		if (backdrop) {
			this.backdropEl = newEl("div", { cls: "modal-backdrop" }, this.el);
			if (closeOnBackdropClick) {
				this.backdropEl.addEventListener("click", (e) => {
					if (e.target === this.backdropEl) this.close();
				});
			}
		}

		if (closeOnEscape) {
			this._onEscape = (e) => {
				if (e.key === "Escape" && this.isOpen()) this.close();
			};
		}

		if (draggable) this._makeDraggable(this.headerEl);
		this._setInitialPosition(x, y);
	}

	_setInitialPosition(x, y) {
		// Default: roughly centered, offset slightly so multiple modals cascade.
		const left = x ?? Math.max(20, (window.innerWidth - (parseInt(this.el.style.width) || 400)) / 2);
		const top = y ?? Math.max(20, window.innerHeight * 0.1);
		this.el.style.left = `${left}px`;
		this.el.style.top = `${top}px`;
	}

	_makeDraggable(handle) {
		let dragging = false;
		let startX, startY, startLeft, startTop;

		const onPointerDown = (e) => {
			// Ignore drags started on the close button.
			if (e.target.closest(".modal-close")) return;
			dragging = true;
			const rect = this.el.getBoundingClientRect();
			startX = e.clientX;
			startY = e.clientY;
			startLeft = rect.left;
			startTop = rect.top;
			this.el.classList.add("modal--dragging");
			handle.setPointerCapture(e.pointerId);
		};

		const onPointerMove = (e) => {
			if (!dragging) return;
			const dx = e.clientX - startX;
			const dy = e.clientY - startY;
			const maxLeft = window.innerWidth - this.el.offsetWidth;
			const maxTop = window.innerHeight - this.el.offsetHeight;
			const left = Math.min(Math.max(0, startLeft + dx), Math.max(0, maxLeft));
			const top = Math.min(Math.max(0, startTop + dy), Math.max(0, maxTop));
			this.el.style.left = `${left}px`;
			this.el.style.top = `${top}px`;
		};

		const onPointerUp = (e) => {
			dragging = false;
			this.el.classList.remove("modal--dragging");
			try { handle.releasePointerCapture(e.pointerId); } catch (_) {}
		};

		handle.addEventListener("pointerdown", onPointerDown);
		handle.addEventListener("pointermove", onPointerMove);
		handle.addEventListener("pointerup", onPointerUp);
		handle.addEventListener("pointercancel", onPointerUp);
	}

	/** Replace the modal body with the given node. */
	setBody(node) {
		this.bodyEl.innerHTML = "";
		this.bodyEl.appendChild(node);
		return this;
	}

	/** Replace the modal footer with the given node (e.g. a Cancel/Send button row). */
	setFooter(node) {
		this.footerEl.innerHTML = "";
		this.footerEl.appendChild(node);
		this.footerEl.style.display = "";
		return this;
	}

	setTitle(title) {
		this.titleEl.textContent = title;
		return this;
	}

	mount(parent = document.body) {
		parent.appendChild(this.hasBackdrop ? this.backdropEl : this.el);
		return this;
	}

	isOpen() {
		return this.el.classList.contains("modal--open");
	}

	open() {
		this.el.classList.add("modal--open");
		if (this.hasBackdrop) this.backdropEl.classList.add("modal-backdrop--open");
		if (this._onEscape) document.addEventListener("keydown", this._onEscape);
		this.onOpen?.(this);
		return this;
	}

	close() {
		this.el.classList.remove("modal--open");
		if (this.hasBackdrop) this.backdropEl.classList.remove("modal-backdrop--open");
		if (this._onEscape) document.removeEventListener("keydown", this._onEscape);
		this.onClose?.(this);
		return this;
	}

	toggle() {
		this.isOpen() ? this.close() : this.open();
		return this;
	}

	destroy() {
		if (this._onEscape) document.removeEventListener("keydown", this._onEscape);
		(this.hasBackdrop ? this.backdropEl : this.el).remove();
	}
}

